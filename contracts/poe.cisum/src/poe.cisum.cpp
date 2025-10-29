#include "poe.cisum.hpp"

namespace flon {

using std::string;

// 转账内部函数（安全封装）
void poe_cisum::_pay_points( const claim_s & claim,const asset& quant ,const uint64_t& uid, const time_point& now ) {
    // 1. 获取奖励配置

    CHECKC(quant.amount > 0 && quant.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "invalid base reward");

    // 2. 预构造 memo
    const string memo = "poe:" + claim.reward_code.to_string() + ":" + claim.memo;
    CHECKC(memo.size() <= 256, err::INVALID_FORMAT, "memo too long");

    // 3. 余额检查与更新
    CHECKC(_gstate.available_points >= quant, err::INSUFFICIENT_QUANTITY, "insufficient available_points");
    _gstate.available_points    -= quant;
    _gstate.claimed_points      += quant;

    auto to = claim.beneficiary;
    CHECKC( is_account(to), err::ACCOUNT_INVALID, "invalid recipient account" + to.to_string() )
    CHECKC( quant.amount > 0 && quant.is_valid(), err::INVALID_FORMAT, "invalid transfer amount" )
    CHECKC( memo.size() <= 256, err::INVALID_FORMAT, "memo too long" )

    TRANSFER(CISUM_BANK, to, quant, memo)

    notifyreward_action{
        get_self(),
        {permission_level{get_self(), "active"_n}}
    }.send(CISUM_BANK, claim.beneficiary, quant, memo, claim.reward_code,std::to_string(uid), now.time_since_epoch().count() / 1'000'000);
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

    // 1. UID 去重
    uid_index uidtable(get_self(), get_self().value);
    auto byuid = uidtable.get_index<"byuid"_n>();
    CHECKC( byuid.find(uid) == byuid.end(), err::RECORD_FOUND, "duplicate uid: already processed " + std::to_string(uid) )
    uidtable.emplace(get_self(), [&](auto& row) {
        row.id = uidtable.available_primary_key();
        row.uid = uid;
        row.created_at = now;
    });
    while (std::distance(uidtable.begin(), uidtable.end()) > 10000)
        uidtable.erase(uidtable.begin());

    rewardact_t::acts_idx acts(get_self(), get_self().value);
    auto byact = acts.get_index<"byname"_n>();

    // 2. 发放奖励与通知
    for (const auto& claim : claims) {
        auto it = byact.find(claim.reward_code.value);
        CHECKC(it != byact.end(), err::RECORD_NO_FOUND, "rewardact not found by reward_code: " + claim.reward_code.to_string());
        const asset quant = it->points;
        _pay_points( claim, quant, uid, now);
    }
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