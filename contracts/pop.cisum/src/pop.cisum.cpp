#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include "pop.cisum.hpp"
#include <string>

namespace flon {

using namespace eosio;
using std::string;

void pop_cisum::mine( name   payer,
                      asset  pay_amount,
                      string memo )
{
  require_auth(get_self());

  CHECKC(is_account(payer), err::ACCOUNT_INVALID, "payer account not exists");
  CHECKC(pay_amount.symbol == USDT_SYMBOL, err::INVALID_FORMAT, "pay_amount symbol mismatch");
  CHECKC(pay_amount.amount > 0, err::INVALID_FORMAT, "pay_amount must be positive");
  CHECKC(memo.size() <= 256, err::INVALID_FORMAT, "memo has more than 256 bytes");

  CHECKC(is_account(_gstate.reward_contract), err::ACCOUNT_INVALID, "reward contract account not exists");

  asset price =  get_price_from_swap_as_asset(REWARD_SYMBOL,USDT_SYMBOL);
  CHECKC(price.amount > 0, err::INVALID_FORMAT, "invalid price from swap");

  // —— 10% USDT 作为奖励等值折算成 CISUM ——
  // reward_usdt_min_units = floor(pay_amount.amount * 10%)
  int64_t reward_usdt_min_units = pay_amount.amount / 10;

  // CISUM 数量 = reward_usdt * (10^CISUM_precision) / price.amount
  // price.amount 表示 1e(CISUM) 对应多少 USDT
  const int64_t p10C = pow10((uint8_t)REWARD_SYMBOL.precision());
  __int128 num = (__int128)reward_usdt_min_units * (__int128)p10C;
  int64_t reward_cisum_min_units = (int64_t)(num / (__int128)price.amount);
  asset reward{ reward_cisum_min_units, REWARD_SYMBOL };

  CHECKC(reward.amount > 0, err::NOT_POSITIVE, "reward too small");
  CHECKC(_gstate.available_rewards.symbol == REWARD_SYMBOL, err::SYMBOL_MISMATCH, "available_rewards symbol mismatch");
  CHECKC(_gstate.issued_rewards.symbol    == REWARD_SYMBOL, err::SYMBOL_MISMATCH, "issued_rewards symbol mismatch");
  CHECKC(_gstate.available_rewards.amount >= reward.amount, err::INSUFFICIENT_QUANTITY, "insufficient available rewards");

  _gstate.available_rewards -= reward;
  _gstate.issued_rewards    += reward;

  // —— 记录应收（USDT）：receivable += pay_amount * 10% ——
  // receivable 以 USDT 计价
  if (_gstate.receivable.symbol.code().raw() == 0) {
    _gstate.receivable = asset(0, USDT_SYMBOL);
  }
  CHECKC(_gstate.receivable.symbol == USDT_SYMBOL, err::SYMBOL_MISMATCH, "receivable symbol mismatch");
  _gstate.receivable += asset(reward_usdt_min_units, USDT_SYMBOL);

  _global.set(_gstate, get_self());

  // —— 发放奖励（从 reward_contract 转账 CISUM 给 payer）——
  action(
    permission_level{ get_self(), "active"_n },
    _gstate.reward_contract,
    "transfer"_n,
    std::make_tuple(get_self(), payer, reward, std::string("pop reward: ") + memo)
  ).send();
}

//memo： order:12345
void pop_cisum::settle(const asset& amount, const string& memo) {
  require_auth(get_self());

  CHECKC(amount.symbol == USDT_SYMBOL, err::SYMBOL_MISMATCH, "only USDT allowed");
  CHECKC(amount.amount > 0, err::NOT_POSITIVE, "settle amount must be positive");

  if (_gstate.receivable.symbol.code().raw() == 0) {
    _gstate.receivable = asset(0, USDT_SYMBOL);
  }
  CHECKC(_gstate.receivable.symbol == USDT_SYMBOL, err::SYMBOL_MISMATCH, "receivable symbol mismatch");
  CHECKC(_gstate.receivable.amount >= amount.amount, err::INSUFFICIENT_QUANTITY, "settle amount exceeds receivable");

  _gstate.receivable -= amount;
  _global.set(_gstate, get_self());

  // 这里只记录账务，真实 USDT 入账应由你们清算侧完成并调用本动作配平
  if (memo.size() > 0) {
    print("settled: ", amount, " memo: ", memo);
  }
}


} /// namespace flon
