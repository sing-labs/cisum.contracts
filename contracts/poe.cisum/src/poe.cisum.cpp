#include "poe.cisum.hpp"

namespace flon {

using std::string;

// 转账内部函数（安全封装）
void poe_cisum::_pay_points( const claim_s & claim, const asset& quant, const uint64_t& uid, const time_point& now ) {
    // 1. 获取奖励配置
    CHECKC(quant.amount > 0 && quant.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "invalid quant");

    // 2. 余额检查与更新
    CHECKC(_gstate.available_points >= quant, err::INSUFFICIENT_QUANTITY, "insufficient available_points");
    _gstate.available_points    -= quant;
    _gstate.claimed_points      += quant;

    const auto to               = claim.beneficiary;
    const string memo           = "poe:" + claim.reward_code.to_string() + ":" + claim.memo;

    TRANSFER( CISUM_BANK, to, quant, memo )

    notifyreward_action{
        get_self(),
        { permission_level{get_self(), "active"_n} }
    }.send(CISUM_BANK, to, quant, memo, claim.reward_code, std::to_string(uid), now.time_since_epoch().count() / 1'000'000);
}

/*─────────────────────────────────────────────────────────────*
 |                        管理操作区                           |
 *─────────────────────────────────────────────────────────────*/

// 添加/更新奖励配置
void poe_cisum::addrewardact(const name& reward_code, const asset& points, const string& description) {
    require_auth(get_self());
    CHECKC(reward_code.value != 0, err::INVALID_FORMAT, "reward_code cannot be empty");
    CHECKC(points.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "points symbol mismatch");
    CHECKC(points.amount >= 0, err::INVALID_FORMAT, "points must be non-negative");
    CHECKC(description.size() <= 256, err::INVALID_FORMAT, "description too long");

    rewardact_t::acts_idx acts(get_self(), get_self().value);
    auto byname = acts.get_index<"byname"_n>();
    auto it = byname.find(reward_code.value);
    const auto now = current_time_point();

    if (it == byname.end()) {
        acts.emplace(get_self(), [&](auto& row) {
            row.id             = ++_gstate.last_act_id;
            row.reward_code    = reward_code;
            row.points         = points;
            row.description    = description;
            row.create_at      = now;
            row.update_at      = now;
        });
    } else {
        byname.modify(it, same_payer, [&](auto& row) {
            row.points          = points;
            row.description     = description;
            row.update_at       = now;
        });
    }
}

// 删除奖励配置（若已发放禁止删除）
void poe_cisum::delrewardact(const name& reward_code) {
    require_auth(get_self());
    rewardact_t::acts_idx acts(get_self(), get_self().value);
    auto byname = acts.get_index<"byname"_n>();
    auto it = byname.find(reward_code.value);
    CHECKC(it != byname.end(), err::RECORD_NO_FOUND, "act not found: " + reward_code.to_string());
    byname.erase(it);
}

// 添加 / 移除 operator
void poe_cisum::addoperator(const name& account) {
    require_auth(get_self());
    CHECKC(account.value != 0 && is_account(account), err::ACCOUNT_INVALID, "invalid operator account");
    _gstate.operators.insert(account);
}
void poe_cisum::deloperator(const name& account) {
    require_auth(get_self());
    CHECKC(account.value != 0, err::ACCOUNT_INVALID, "operator account cannot be empty");
    auto it = _gstate.operators.find(account);
    if (it != _gstate.operators.end()) {
        _gstate.operators.erase(it);
    }
}

/*─────────────────────────────────────────────────────────────*
 |                        发放奖励区                           |
 *─────────────────────────────────────────────────────────────*/

void poe_cisum::batchclaim(const name& claimer,
                           const uint64_t& uid,
                           const std::vector<claim_s>& claims) {
    require_auth(claimer);
    CHECKC(_gstate.operators.count(claimer) > 0, err::DID_NOT_AUTH,
           "claimer not in operators whitelist: " + claimer.to_string());
    CHECKC(!claims.empty(), err::INVALID_FORMAT, "empty claim list");
    CHECKC(claims.size() <= 100, err::EXCEED_LIMIT,
       "too many claims in batch (max 100, got " + std::to_string(claims.size()) + ")");
    const auto now = current_time_point();

    rewardact_t::acts_idx acts(get_self(), get_self().value);
    auto byact = acts.get_index<"byname"_n>();

    // 2. 发放奖励与通知
    for (const auto& claim : claims) {
        if (!is_account(claim.beneficiary)) continue;
        // 检查奖励类型是否存在
        auto it = byact.find(claim.reward_code.value);
        CHECKC(it != byact.end(), err::RECORD_NO_FOUND, "rewardact not found by: " + claim.reward_code.to_string());
        
        const auto quant = it->points;
        _pay_points( claim, quant, uid, now);
    }
}

/*─────────────────────────────────────────────────────────────*
 |                        批量空投                              |
 *─────────────────────────────────────────────────────────────*/
void poe_cisum::batchairdrop(const name& oper, const std::vector<airdrop_s>& drops, const string& memo) {
    require_auth(oper);
    CHECKC(_gstate.operators.count(oper) > 0, err::DID_NOT_AUTH, "oper not in operators whitelist");
    CHECKC(!drops.empty(), err::INVALID_FORMAT, "empty airdrop list");
    CHECKC(drops.size() <= 1000, err::EXCEED_LIMIT, "too many airdrops (max 1000)");

    CHECKC(memo.size() <= 256, err::INVALID_FORMAT, "memo too long");

    // 转账前校验整批数据；asset 加法同时检查累计金额是否超出合法范围。
    asset total(0, CISUM_SYM);
    for (const auto& drop : drops) {
        CHECKC(is_account(drop.beneficiary) && drop.beneficiary != get_self(),
               err::ACCOUNT_INVALID, "beneficiary must exist and cannot be self");
        CHECKC(drop.quantity.is_valid() && drop.quantity.symbol == CISUM_SYM,
               err::SYMBOL_MISMATCH, "quantity must be valid CISUM with precision 4");
        CHECKC(drop.quantity.amount > 0, err::NOT_POSITIVE, "quantity must be positive");
        CHECKC(drop.quantity.amount < 1000000000, err::EXCEED_LIMIT, "quantity must be less than 100000.0000 CISUM");
        total += drop.quantity;
    }

    // 只使用已充值的 CISUM，不调用 issue；余额不足时整批拒绝。
    const auto balance = token::get_balance(CISUM_BANK, get_self(), CISUM_SYM.code());
    eosio::check(balance >= total, "insufficient CISUM balance");
    // 与 batchclaim 共用奖励池账目，防止转出后可用积分仍然虚高。
    CHECKC(_gstate.available_points >= total, err::INSUFFICIENT_QUANTITY, "insufficient available_points");
    _gstate.available_points -= total;
    _gstate.claimed_points += total;
    // available + claimed 不变，因此 total_points 不需要调整。
    const auto created_at = current_time_point().time_since_epoch().count() / 1'000'000;
    for (const auto& drop : drops) {
        TRANSFER(CISUM_BANK, drop.beneficiary, drop.quantity, memo);
        notifyreward_action{
            get_self(), { permission_level{get_self(), "active"_n} }
        }.send(CISUM_BANK, drop.beneficiary, drop.quantity, memo,
               "airdrop"_n, "", created_at);
    }
    // 任一内联转账失败，整笔交易（包括累计发放量）一起回滚。
    // 不做业务去重：重复提交同一批数据会再次从余额发放。
}

// 发放通知（dummy action，用于 require_recipient）
void poe_cisum::notifyreward(const name& from,
                            const name& to,
                            const asset& award_amount,
                            const string& memo,
                            const name& reward_type,
                            const string& reward_ref_id,
                            const uint64_t& created_at) {
    require_auth(get_self());
}


/*─────────────────────────────────────────────────────────────*
 |                        积分管理区                           |
 *─────────────────────────────────────────────────────────────*/

// 消耗积分（被其他合约调用）
void poe_cisum::consumeact(const name& submitter, const name& reward_code, const asset& amount) {
    CHECKC(has_auth(get_self()) || has_auth(POH_CONTRACT) || _gstate.operators.count(submitter) > 0,
           err::DID_NOT_AUTH, "not authorized");

    CHECKC(reward_code.value != 0, err::INVALID_FORMAT, "reward_code is empty");
    CHECKC(amount.is_valid(), err::INVALID_FORMAT, "invalid amount");
    CHECKC(amount.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "symbol mismatch");
    CHECKC(amount.amount >= 0, err::NOT_POSITIVE, "amount must be non-negative");

    rewardact_t::acts_idx acts(get_self(), get_self().value);
    auto byname = acts.get_index<"byname"_n>();
    auto it = byname.find(reward_code.value);
    CHECKC(it != byname.end(), err::RECORD_NO_FOUND, "act not found: " + reward_code.to_string());

    CHECKC(_gstate.available_points.amount >= amount.amount, err::INSUFFICIENT_QUANTITY, "insufficient balance");

    CHECKC(_gstate.claimed_points.amount <= std::numeric_limits<int64_t>::max() - amount.amount,
           err::AMOUNT_TOO_LARGE, "global claimed_points overflow");

    _gstate.available_points.amount -= amount.amount;
    _gstate.claimed_points.amount += amount.amount;

    const int64_t avail = _gstate.available_points.amount;
    const int64_t claimed = _gstate.claimed_points.amount;
    CHECKC(avail <= std::numeric_limits<int64_t>::max() - claimed, err::AMOUNT_TOO_LARGE, "total_points overflow");
    _gstate.total_points.amount = avail + claimed;

    // byname.modify(it, same_payer, [&](auto& r) {
    //     r.claimed_points.amount += amount.amount;
    //     r.update_at = current_time_point();
    // });
}

/*─────────────────────────────────────────────────────────────*
 |                        充值入口                             |
 *─────────────────────────────────────────────────────────────*/

void poe_cisum::ontransfer(const name& from, const name& to, const asset& quantity, const string& memo) {
    if (from == get_self() || to != get_self()) return;

    CHECKC(get_first_receiver() == CISUM_CONTRACT, err::INVALID_FORMAT, "invalid token contract");
    CHECKC(quantity.symbol == CISUM_SYM && quantity.is_valid(), err::SYMBOL_MISMATCH, "invalid symbol or format");
    CHECKC(quantity.amount > 0, err::NOT_POSITIVE, "must transfer positive");

    // 初始化符号
    if (_gstate.available_points.symbol.code().raw() == 0)
        _gstate.available_points = asset(0, CISUM_SYM);
    if (_gstate.claimed_points.symbol.code().raw() == 0)
        _gstate.claimed_points = asset(0, CISUM_SYM);
    if (_gstate.total_points.symbol.code().raw() == 0)
        _gstate.total_points = asset(0, CISUM_SYM);

    // 防溢出更新
    CHECKC(_gstate.available_points.amount <= std::numeric_limits<int64_t>::max() - quantity.amount,
           err::AMOUNT_TOO_LARGE, "available_points overflow");
    _gstate.available_points.amount += quantity.amount;

    const int64_t avail = _gstate.available_points.amount;
    const int64_t claimed = _gstate.claimed_points.amount;
    CHECKC(avail <= std::numeric_limits<int64_t>::max() - claimed, err::AMOUNT_TOO_LARGE, "total_points overflow");
    _gstate.total_points.amount = avail + claimed;
}

} // namespace flon