#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <optional>
#include <string>

#include "pop.cisum.db.hpp"

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
  [[eosio::action]]
  void mine(  name        payer,
              asset       pay_amount,
              string      memo
   );

  // -------- Inline wrappers --------
  using mine_action        = eosio::action_wrapper<"mine"_n,        &pop_cisum::mine>;

private:
  global_singleton _global;
  global_t         _gstate;

};

} // namespace flon