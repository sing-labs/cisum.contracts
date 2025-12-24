#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <vector>

#include "cisumshowman.db.hpp"

namespace flon {

class [[eosio::contract("cisumshowman")]] cisumshow : public eosio::contract {
public:
  using contract::contract;

    cisumshow(eosio::name receiver, eosio::name code, datastream<const char*> ds)
  : contract(receiver, code, ds)
    //_global(get_self(), get_self().value)
  {
    //_gstate = _global.exists() ? _global.get() : global_t{};
  }

  ~cisumshow() { //_global.set(_gstate, get_self());
  }

  // 一次性：newshow -> (createnft -> newticket -> issuenft)* -> 若免费票则发给 grap.cisum
  ACTION publishshow(eosio::name creator,
                   const show_info& show,
                   const std::vector<ticket_info>& tickets);

  ACTION addupgrades(const name& creator,const uint64_t&   show_id,const vector<ticket_info>& tickets,const name& activity_type);


  using publishshow_action = eosio::action_wrapper<"publishshow"_n, &cisumshow::publishshow>;

// private:
//   global_singleton _global;
//   global_t         _gstate;


};


} // namespace flon