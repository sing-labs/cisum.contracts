#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <optional>
#include <string>

#include "pop.cisum.db.hpp"

#include "flon.swap/flon.swap.db.hpp"

using std::string;
using namespace eosio;

namespace flon {

class [[eosio::contract("pop.cisum")]] pop_cisum : public contract {
public:
  using contract::contract;

  pop_cisum(eosio::name receiver, eosio::name code, datastream<const char*> ds)
  : contract(receiver, code, ds),
    _global(get_self(), get_self().value)
  {
    _gstate = _global.exists() ? _global.get() : global_t{};
  }

  ~pop_cisum() { _global.set(_gstate, get_self()); }

  /**
   * Mine rewards for the payment.
   * Requires contract self authorization.
   *
   * @param payer        The payer account who will receive rewards.
   * @param pay_amount   The pay amount.
   * @param memo         Additional information or remarks.
   */


  ACTION notifyreward(const name&  from,
                                const name&       to,
                                const asset&      award_amount,
                                const string&     memo,
                                const name&       reward_type,
                                const string&     reward_ref_id,
                                const uint64_t&   created_at);

  ACTION init(const asset& max_rewards);

  ACTION mine(  name  payer,asset pay_amount,string  memo);

  ACTION  settle(const asset& amount, const string& memo) ;

  ACTION setmaxreward(const asset& max_rewards);

  ACTION addexecutor(const name& acct);
  ACTION delexecutor(const name& acct);

  ACTION setrule(uint64_t id, const asset& threshold, const nsymbol& symbol, bool enabled) ;

  ACTION delrule(uint64_t id);

  ACTION notifyaward(const name& user,
                                  const vector<nasset>& packs,
                                  const string& memo);

  // -------- Inline wrappers --------
  using mine_action                       = eosio::action_wrapper<"mine"_n,&pop_cisum::mine>;
  using notifyreward_action                = eosio::action_wrapper<"notifyreward"_n,&pop_cisum::notifyreward>;
  using notifyaward_action                = eosio::action_wrapper<"notifyaward"_n,&pop_cisum::notifyaward>;
private:
  global_singleton _global;
  global_t         _gstate;


private:
  // 小写化 symbol_code -> std::string
  static std::string to_lower(const symbol_code& sc) {
    std::string s = sc.to_string();
    for (char& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
  }

  // 10^p（0<=p<=18）
  static int64_t pow10(int p) {
    int64_t v = 1;
    while (p-- > 0) v *= 10;
    return v;
  }

  // 从 flon.swap 读取池子并返回按 left 精度放大的 "quote per left"
  inline asset get_price_from_swap_as_asset(const symbol& left_sym,
                                            const symbol& right_sym)
  {
    const name swap_ctr = "flon.swap"_n;
    const name tpcode{ to_lower(left_sym.code()) + "." + to_lower(right_sym.code()) };

    flon::market_t::idx_t markets(swap_ctr, swap_ctr.value);
    auto it = markets.find(tpcode.value);
    check(it != markets.end(), "market not found: " + tpcode.to_string());

    const asset& L = it->left_pool_quant.quantity;   // left 池量
    const asset& R = it->right_pool_quant.quantity;  // right 池量
    check(L.symbol == left_sym && R.symbol == right_sym, "symbol mismatch in market");
    check(L.amount > 0 && R.amount > 0, "empty pool");

    const int64_t scale = pow10(L.symbol.precision());

    __int128 num = (__int128)R.amount * (__int128)scale;
    int64_t price_amount = (int64_t)(num / (__int128)L.amount); // floor

    return asset{ price_amount, right_sym };
  }

  void _try_award_badges(const name& user,
                            int64_t consumed_before,
                            int64_t consumed_after);


};
} // namespace flon