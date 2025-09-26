#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include "pop.cisum.hpp"
#include <string>
#include <flon/flon.token.hpp>
#include <flon/consts.hpp>

namespace flon {

using namespace eosio;
using std::string;

void pop_cisum::awardnotice(const name&  from,
                                const name&       to,
                                const asset&      award_amount,
                                const string&     memo,
                                const name&       reward_type,
                                const string&     reward_ref_id,
                                const uint64_t&   created_at)
{
     require_auth(get_self());
}


void pop_cisum::notifyaward(const name& user, const vector<nasset>& packs, const string& memo) {
    require_auth(get_self());
    require_recipient(CVBADGESTORE_CONTRACT);
}


static inline std::string build_award_memo(const eosio::name& user,
                                           const std::vector<nasset>& packs) {
    std::string prefix = "pop.cisum|auto-award|" + user.to_string();
    std::string memo   = prefix;

    // 紧凑追加：|101x3|102x1 ...
    for (const auto& p : packs) {
        memo += "|" + std::to_string(p.symbol.id()) + "x" + std::to_string(p.amount);

        // 留点余量，超长就用总量兜底
        if (memo.size() >= 250) {
            int64_t total = 0;
            for (const auto& q : packs) total += q.amount;
            memo = prefix + "|amt=" + std::to_string(total);
            break;
        }
    }

    check(memo.size() <= 256, "memo too long");
    return memo;
}

void pop_cisum::_try_award_badges(const name &user, int64_t consumed_before, int64_t consumed_after)
{
    if (consumed_after <= consumed_before) return;

    badge_rule_t::idx_t rules(get_self(), get_self().value);
    if (rules.begin() == rules.end()) return;

    std::vector<nasset> packs;

    for (auto it = rules.begin(); it != rules.end(); ++it) {
        if (!it->enabled) continue;
        if (it->threshold.symbol != USDT_SYM) continue;

        const int64_t step = it->threshold.amount;
        if (step <= 0) continue;

        const int64_t k_before = consumed_before / step;
        const int64_t k_after  = consumed_after  / step;
        const int64_t delta    = k_after - k_before;

        if (delta > 0) {
        packs.emplace_back(delta, it->symbol);
        }
    }

    if (packs.empty()) return;

    std::string memo = build_award_memo(user, packs);

    notifyaward_action{
        get_self(),
        { permission_level{ get_self(), "active"_n } }
    }.send(user, packs, memo);
}




void pop_cisum::mine(name payer, asset pay_amount, string memo)
{
    // ===== 0) 授权：合约自身 或 白名单任一账号 =====
    bool authed = has_auth(get_self());
    if (!authed) {
        for (const auto& ex : _gstate.executors) {
        if (has_auth(ex)) { authed = true; break; }
        }
    }
    check(authed, "executor or self not authorized");

    // —— 基础校验 ——
    CHECKC(payer.value != 0 && is_account(payer),   err::ACCOUNT_INVALID, "payer account not exists");
    CHECKC(pay_amount.symbol == USDT_SYM,           err::SYMBOL_MISMATCH, "pay_amount symbol must be USDT");
    CHECKC(pay_amount.is_valid(),                   err::INVALID_FORMAT,  "invalid pay_amount");
    CHECKC(pay_amount.amount > 0,                   err::NOT_POSITIVE,    "pay_amount must be positive");
    CHECKC(memo.size() <= 256,                      err::INVALID_FORMAT,  "memo too long");
    CHECKC(is_account(_gstate.reward_contract),     err::ACCOUNT_INVALID, "reward contract not exists");

    // —— 确保全局资产的符号初始化正确 ——
    if (_gstate.available_rewards.symbol.code().raw() == 0) _gstate.available_rewards = asset(0, SING_SYM);
    if (_gstate.issued_rewards.symbol.code().raw()    == 0) _gstate.issued_rewards    = asset(0, SING_SYM);
    if (_gstate.max_rewards.symbol.code().raw()       == 0) _gstate.max_rewards       = MAX_REWARDS_DEFAULT;

    CHECKC(_gstate.available_rewards.symbol == SING_SYM, err::SYMBOL_MISMATCH, "available_rewards symbol mismatch");
    CHECKC(_gstate.issued_rewards.symbol    == SING_SYM, err::SYMBOL_MISMATCH, "issued_rewards symbol mismatch");
    CHECKC(_gstate.max_rewards.symbol       == SING_SYM, err::SYMBOL_MISMATCH, "max_rewards symbol mismatch");

    // —— 价格：SING/USDT （1eSING 对应多少 USDT 的最小单位）——
    asset price = get_price_from_swap_as_asset(SING_SYM, USDT_SYM);
    CHECKC(price.is_valid() && price.symbol == USDT_SYM, err::INVALID_FORMAT, "invalid price from swap");
    CHECKC(price.amount > 0,                                 err::INVALID_FORMAT, "price must be positive");

    // —— 计算奖励：10% USDT 折算为 SING ——
    // reward_usdt_min_units = floor( pay_amount * 10% )，以 USDT 最小单位计
    const int64_t reward_usdt_min_units = pay_amount.amount / 10;

    // reward_sing_min_units = reward_usdt_min_units * 10^sing_precision / price.amount
    const int64_t p10C = pow10((uint8_t)SING_SYM.precision());
    __int128 num       = (__int128)reward_usdt_min_units * (__int128)p10C;
    int64_t reward_sing_min_units = (int64_t)(num / (__int128)price.amount);

    asset reward{ reward_sing_min_units, SING_SYM };
    if (reward.amount <= 0) {
        // 奖励过小 -> 不发，不改状态
        print("[pop] reward=0, skip mint for ", payer);
        return;
    }

    // —— 是否可以发放（不抛错，超限则静默跳过）——
    bool can_issue = true;

    // 1) 池子是否足够
    if (_gstate.available_rewards.amount < reward.amount) {
        can_issue = false;
    }

    // 2) issued_rewards 加上本次不会溢出、不会超过 max_rewards
    if (can_issue) {
        if (_gstate.issued_rewards.amount > std::numeric_limits<int64_t>::max() - reward.amount) {
            can_issue = false; // 防止 int64 溢出
        } else if (_gstate.issued_rewards.amount + reward.amount > _gstate.max_rewards.amount) {
            can_issue = false; // 超过最大发放上限
        }
    }

    if (!can_issue) {
        // 按你的要求：不发、不回滚、不更新任何状态
        print("[pop] cap reached or insufficient pool, skip mint for ", payer,
              ", reward ", reward.to_string());
        return;
    }

    // —— 更新状态（只在可发时）——
    _gstate.available_rewards -= reward;
    _gstate.issued_rewards    += reward;

    // reward_usdt_accum 仅在实际铸发时累加（10% USDT）
    if (_gstate.reward_usdt_accum.symbol.code().raw() == 0) _gstate.reward_usdt_accum = asset(0, USDT_SYM);
    CHECKC(_gstate.reward_usdt_accum.symbol == USDT_SYM, err::SYMBOL_MISMATCH, "reward_usdt_accum symbol mismatch");
    CHECKC(_gstate.reward_usdt_accum.amount <= std::numeric_limits<int64_t>::max() - reward_usdt_min_units,
           err::AMOUNT_TOO_LARGE, "reward_usdt_accum overflow");
    _gstate.reward_usdt_accum.amount += reward_usdt_min_units;

    _global.set(_gstate, get_self());

    // —— 动态铸币 & 发放（使用你的宏）——
    ISSUE(
        _gstate.reward_contract,              // 银行合约
        _self,                                // 先铸到本合约
        reward,
        std::string("Purchase Reward")
    );
    TRANSFER(
        _gstate.reward_contract,              // 从银行合约转账
        payer,                               // 发给付款人
        reward,
        std::string("Purchase Reward(")+payer.to_string()+")"
    );

    awardnotice_action{
            get_self(),
            { permission_level{ get_self(), "active"_n } }
    }.send(
        _gstate.reward_contract,
        payer,
        reward,
        std::string("Purchase Reward(")+payer.to_string()+")",
        "purchasemint",
        memo,
        current_time_point().time_since_epoch().count() / 1'000'000
    );


    // ===== 累计消费 =====
    receivable_singleton recv_tbl(get_self(), payer.value);
    receivable_t recv;

    if (recv_tbl.exists()) {
        recv = recv_tbl.get();
    } else {
        recv.amount    = asset(0, USDT_SYM);
        recv.updated_at = current_time_point();
    }

    CHECKC(recv.amount.symbol == pay_amount.symbol, err::SYMBOL_MISMATCH, "receivable symbol mismatch");
    int64_t before = recv.amount.amount;
    recv.amount   += pay_amount;
    recv.updated_at = current_time_point();

    int64_t after = recv.amount.amount;

    recv_tbl.set(recv, get_self());

    // ===== 判断是否跨过勋章门槛 =====
    _try_award_badges(payer, before, after);


}

//memo： order:12345
void pop_cisum::settle(const asset& amount, const string& memo) {
    require_auth(get_self());

    CHECKC(amount.symbol == USDT_SYM, err::SYMBOL_MISMATCH, "only USDT allowed");
    CHECKC(amount.amount > 0, err::NOT_POSITIVE, "settle amount must be positive");
    CHECKC(memo.size() <= 256, err::INVALID_FORMAT, "memo too long");

    if (_gstate.reward_usdt_accum.symbol.code().raw() == 0) {
      _gstate.reward_usdt_accum = asset(0, USDT_SYM);
    }
    CHECKC(_gstate.reward_usdt_accum.symbol == USDT_SYM, err::SYMBOL_MISMATCH, "reward_usdt_accum symbol mismatch");
    CHECKC(_gstate.reward_usdt_accum.amount >= amount.amount, err::INSUFFICIENT_QUANTITY, "settle amount exceeds reward_usdt_accum");

    _gstate.reward_usdt_accum -= amount;
    _global.set(_gstate, get_self());
}

void pop_cisum::setmaxreward(const asset& max_rewards) {
    require_auth(get_self());

    check(max_rewards.is_valid(),       "invalid max_rewards");
    check(max_rewards.symbol == SING_SYM, "symbol mismatch for max_rewards");
    check(max_rewards.amount > 0,       "max_rewards must be positive");

    _gstate.max_rewards       = max_rewards;
    if (_gstate.available_rewards.amount > max_rewards.amount) {
        // 如果当前可用奖励超过新上限，则强行截断
        _gstate.available_rewards.amount = max_rewards.amount;
    }
    _global.set(_gstate, get_self());
}

void pop_cisum::addexecutor(const name& acct) {
  require_auth(get_self());

  check(acct.value != 0, "invalid account (empty name)");
  check(is_account(acct), "invalid account (not exist)");
  // 可选：不要把自己加进去（一般没意义）
  check(acct != get_self(), "executor cannot be contract itself");

  // 幂等：已存在就直接返回
  if (_gstate.executors.find(acct) != _gstate.executors.end()) {
    return;
  }

  _gstate.executors.insert(acct);
  _global.set(_gstate, get_self());
}

void pop_cisum::delexecutor(const name& acct) {
  require_auth(get_self());

  // 幂等：不存在就直接返回（便于脚本重复执行）
  auto it = _gstate.executors.find(acct);
  if (it == _gstate.executors.end()) {
    return;
  }

  _gstate.executors.erase(it);
  _global.set(_gstate, get_self());
}


void pop_cisum::setbrule(uint64_t id, const asset& threshold, const nsymbol& symbol, bool enabled) {
    require_auth(get_self());
    CHECKC(threshold.amount > 0,                 err::NOT_POSITIVE,     "threshold must be positive");
    CHECKC(symbol.raw() != 0,                    err::INVALID_FORMAT,   "badge symbol required");


    badge_rule_t::idx_t rtbl(get_self(), get_self().value);
    auto by_symbol = rtbl.get_index<"bysymbol"_n>();

    if (id == 0) {

        CHECKC(by_symbol.find(symbol.raw()) == by_symbol.end(),
               err::REDPACK_EXIST, "rule for this badge symbol already exists");

        rtbl.emplace(get_self(), [&](auto& r){
            r.id         = rtbl.available_primary_key();
            r.threshold  = threshold;
            r.symbol     = symbol;
            r.enabled    = enabled;
            r.created_at = current_time_point();
        });
    } else {
        auto it = rtbl.find(id);
        CHECKC(it != rtbl.end(), err::RECORD_NO_FOUND, "badge rule not found");

        if (it->symbol.raw() != symbol.raw()) {
            CHECKC(by_symbol.find(symbol.raw()) == by_symbol.end(),
                   err::REDPACK_EXIST, "rule for this badge symbol already exists");
        }

        rtbl.modify(it, same_payer, [&](auto& r){
            r.threshold = threshold;
            r.symbol    = symbol;
            r.enabled   = enabled;
        });
    }
}

void pop_cisum::delbrule(uint64_t id)
{
    require_auth(get_self());
    badge_rule_t::idx_t rtbl(get_self(), get_self().value);
    auto it = rtbl.find(id);
    CHECKC(it != rtbl.end(),          err::RECORD_NO_FOUND, "badge rule not found");
    rtbl.erase(it);
}





} /// namespace flon
