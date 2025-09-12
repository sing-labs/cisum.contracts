#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <optional>
#include <string>

#include "flon.swap/flon.swap.db.hpp"

using std::string;
using namespace eosio;

namespace flon {

class [[eosio::contract("pop.cisum")]] pop_cisum : public contract {
public:
  using contract::contract;

  /**
   * Mine rewards for the payment.
   * Requires contract self authorization.
   *
   * @param payer        The payer account who will receive rewards.
   * @param pay_amount   The pay amount.
   * @param memo         Additional information or remarks.
   */
  ACTION mine(  name  payer,asset pay_amount,string  memo);


  ACTION  settle(const asset& amount, const string& memo) ;

  ACTION setmaxreward(const asset& max_rewards);


  // -------- Inline wrappers --------
  using mine_action        = eosio::action_wrapper<"mine"_n,        &pop_cisum::mine>;

};
} // namespace flon