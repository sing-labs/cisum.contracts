#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <string>

#include "poe.cisum.db.hpp"

using namespace eosio;
using std::string;

namespace flon {

class [[eosio::contract("poe.cisum")]] poe_cisum : public contract {
public:
  using contract::contract;

  poe_cisum(eosio::name receiver, eosio::name code, datastream<const char*> ds)
  : contract(receiver, code, ds),
    _global(get_self(), get_self().value)
  {
    _gstate = _global.exists() ? _global.get() : global_t{};
  }

  ~poe_cisum() { _global.set(_gstate, get_self()); }


  [[eosio::action]]
  void setact(name act_name, asset points, string memo);

  [[eosio::action]]
  void delact(name act_name);

  [[eosio::action]]
  void claimpoints(name claimer, name act_name);

  [[eosio::on_notify("nest21.token::transfer")]]
  void ontransfer(name from, name to, asset quantity, string memo);

  // 便捷别名
  using setact_action     = eosio::action_wrapper<"setact"_n,     &poe_cisum::setact>;
  using delact_action     = eosio::action_wrapper<"delact"_n,     &poe_cisum::delact>;
  using claimpts_action   = eosio::action_wrapper<"claimpoints"_n,&poe_cisum::claimpoints>;

private:
  global_singleton _global;
  global_t         _gstate;

  void _pay_points(const name& to, const asset& quant, const string& memo);

  act_t _get_act(const name& act_name);
};

} // namespace flon