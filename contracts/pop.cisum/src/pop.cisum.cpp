#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include "pop.cisum.hpp"
#include <string>
#include <flon/flon.token.hpp>

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
    if (_gstate.available_rewards.symbol.code().raw() == 0) _gstate.available_rewards = asset(0, CISUM_SYM);
    if (_gstate.issued_rewards.symbol.code().raw()    == 0) _gstate.issued_rewards    = asset(0, CISUM_SYM);
    if (_gstate.max_rewards.symbol.code().raw()       == 0) _gstate.max_rewards       = MAX_REWARDS_DEFAULT;

    CHECKC(_gstate.available_rewards.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "available_rewards symbol mismatch");
    CHECKC(_gstate.issued_rewards.symbol    == CISUM_SYM, err::SYMBOL_MISMATCH, "issued_rewards symbol mismatch");
    CHECKC(_gstate.max_rewards.symbol       == CISUM_SYM, err::SYMBOL_MISMATCH, "max_rewards symbol mismatch");

    // —— 价格：CISUM/USDT （1eCISUM 对应多少 USDT 的最小单位）——
    asset price = get_price_from_swap_as_asset(CISUM_SYM, USDT_SYM);
    CHECKC(price.is_valid() && price.symbol == USDT_SYM, err::INVALID_FORMAT, "invalid price from swap");
    CHECKC(price.amount > 0,                                 err::INVALID_FORMAT, "price must be positive");

    // —— 计算奖励：10% USDT 折算为 CISUM ——
    // reward_usdt_min_units = floor( pay_amount * 10% )，以 USDT 最小单位计
    const int64_t reward_usdt_min_units = pay_amount.amount / 10;

    // reward_cisum_min_units = reward_usdt_min_units * 10^cisum_precision / price.amount
    const int64_t p10C = pow10((uint8_t)CISUM_SYM.precision());
    __int128 num       = (__int128)reward_usdt_min_units * (__int128)p10C;
    int64_t reward_cisum_min_units = (int64_t)(num / (__int128)price.amount);

    asset reward{ reward_cisum_min_units, CISUM_SYM };
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

    // receivable 仅在实际铸发时累加（10% USDT）
    if (_gstate.receivable.symbol.code().raw() == 0) _gstate.receivable = asset(0, USDT_SYM);
    CHECKC(_gstate.receivable.symbol == USDT_SYM, err::SYMBOL_MISMATCH, "receivable symbol mismatch");
    CHECKC(_gstate.receivable.amount <= std::numeric_limits<int64_t>::max() - reward_usdt_min_units,
           err::AMOUNT_TOO_LARGE, "receivable overflow");
    _gstate.receivable.amount += reward_usdt_min_units;

    _global.set(_gstate, get_self());

    // —— 动态铸币 & 发放（使用你的宏）——
    ISSUE(
        _gstate.reward_contract,              // 银行合约
        _self,                                // 先铸到本合约
        reward,
        std::string("Purchase Reward: ") + payer.to_string()
    );
    TRANSFER(
        _gstate.reward_contract,              // 从银行合约转账
        payer,                               // 发给付款人
        reward,
        std::string("type:Purchase Reward|") + "amount: " + reward.to_string()+"|account:"+ payer.to_string()+"|order:"+memo
    );

    awardnotice_action{
            get_self(),
            { permission_level{ get_self(), "active"_n } }
    }.send(
        _gstate.reward_contract,
        payer,
        reward,
        std::string("type:Purchase Reward|") + "amount: " + reward.to_string()+"|account:"+ payer.to_string()+"|order:"+memo,
        "purchasemint",
        memo,
        current_time_point().time_since_epoch().count() / 1'000'000
    );

}

//memo： order:12345
void pop_cisum::settle(const asset& amount, const string& memo) {
    require_auth(get_self());

    CHECKC(amount.symbol == USDT_SYM, err::SYMBOL_MISMATCH, "only USDT allowed");
    CHECKC(amount.amount > 0, err::NOT_POSITIVE, "settle amount must be positive");
    CHECKC(memo.size() <= 256, err::INVALID_FORMAT, "memo too long");

    if (_gstate.receivable.symbol.code().raw() == 0) {
      _gstate.receivable = asset(0, USDT_SYM);
    }
    CHECKC(_gstate.receivable.symbol == USDT_SYM, err::SYMBOL_MISMATCH, "receivable symbol mismatch");
    CHECKC(_gstate.receivable.amount >= amount.amount, err::INSUFFICIENT_QUANTITY, "settle amount exceeds receivable");

    _gstate.receivable -= amount;
    _global.set(_gstate, get_self());
}

void pop_cisum::setmaxreward(const asset& max_rewards) {
    require_auth(get_self());

    check(max_rewards.is_valid(),       "invalid max_rewards");
    check(max_rewards.symbol == CISUM_SYM, "symbol mismatch for max_rewards");
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

} /// namespace flon
