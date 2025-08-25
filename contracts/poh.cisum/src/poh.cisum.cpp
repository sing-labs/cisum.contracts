#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <string>

#include "poh.cisum.hpp"

namespace flon {

using eosio::asset;
using eosio::check;
using eosio::current_time_point;
using eosio::is_account;
using eosio::name;
using std::string;

void poh_cisum::init(name platform, name registrar) {
    require_auth(get_self());

    CHECKC(is_account(platform),  err::ACCOUNT_INVALID, "platform not exist");
    CHECKC(is_account(registrar), err::ACCOUNT_INVALID, "registrar not exist");

    _gstate.platform_acct = platform;
    _gstate.registrar     = registrar;
    _gstate.issued_rewards = asset(0, CISUM_SYM);
    _gstate.available_rewards = _gstate.max_reward - _gstate.issued_rewards;

    _global.set(_gstate, get_self());
}

void poh_cisum::setplatform(name platform) {
    require_auth(get_self());
    CHECKC(is_account(platform), err::ACCOUNT_INVALID, "platform not exist");
    _gstate.platform_acct = platform;
    _global.set(_gstate, get_self());
}

void poh_cisum::setregistrar(name registrar) {
    require_auth(get_self());
    CHECKC(is_account(registrar), err::ACCOUNT_INVALID, "registrar not exist");
    _gstate.registrar = registrar;
    _global.set(_gstate, get_self());
}

void poh_cisum::setrewards(const asset& max_reward) {
    require_auth(get_self());  // 只有合约自身可改
    CHECKC(max_reward.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "max_reward symbol mismatch");
    CHECKC(max_reward.amount > 0,           err::NOT_POSITIVE,   "max_reward must be positive");

    asset new_available = max_reward - _gstate.issued_rewards;
    CHECKC(new_available.amount >= 0, err::INSUFFICIENT_QUANTITY,
           "max_reward is less than already issued amount");

    _gstate.max_reward        = max_reward;
    _gstate.available_rewards = new_available;
    _global.set(_gstate, get_self());
}

// -------------------- 业务动作 --------------------

void poh_cisum::registreward(name user, string memo) {
  CHECKC(_gstate.registrar.value != 0, err::RECORD_NO_FOUND, "registrar not set");
  require_auth(_gstate.registrar);

  CHECKC(is_account(user), err::ACCOUNT_INVALID, "user not exist");
  CHECKC(memo.size() <= 256, err::INVALID_FORMAT, "memo too long");
  CHECKC(_gstate.platform_acct.value != 0, err::RECORD_NO_FOUND, "platform not set");

  claimed_idx ctbl(get_self(), get_self().value);
  auto cit = ctbl.find(user.value);
  CHECKC(cit == ctbl.end(), err::NOT_REPEAT_RECEIVE, "already claimed");
  asset usd = _gstate.usd_per_user;
  CHECKC(usd.symbol == USDT_SYM, err::SYMBOL_MISMATCH, "global usd_per_user symbol mismatch");
  CHECKC(usd.amount > 0,         err::NOT_POSITIVE,    "usd_per_user not positive");

  asset reward = usd_to_cisum(usd);
  CHECKC(reward.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "reward symbol mismatch");
  CHECKC(reward.amount > 0,         err::NOT_POSITIVE,     "reward too small");

  CHECKC(_gstate.available_rewards.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "available_rewards symbol mismatch");
  CHECKC(_gstate.issued_rewards.symbol    == CISUM_SYM, err::SYMBOL_MISMATCH, "issued_rewards symbol mismatch");
  CHECKC(_gstate.max_reward.symbol        == CISUM_SYM, err::SYMBOL_MISMATCH, "max_reward symbol mismatch");

  CHECKC(_gstate.available_rewards.amount >= reward.amount,
        err::INSUFFICIENT_QUANTITY, "insufficient available rewards");

  CHECKC(_gstate.issued_rewards.amount + reward.amount <= _gstate.max_reward.amount,
        err::EXCEED_LIMIT, "exceed max reward cap");

  // 记账（可用-，已发+）
  _gstate.available_rewards -= reward;
  _gstate.issued_rewards    += reward;
  _global.set(_gstate, get_self());

  ctbl.emplace(get_self(), [&](auto& r){
    r.user       = user;
    r.claimed_at = current_time_point();
  });

  // 发到平台账户（平台统一入账/分账）
  pay_reward_to_platform(reward, std::string("poh reward: ") + user.to_string() + " | " + memo);
}

} // namespace flon