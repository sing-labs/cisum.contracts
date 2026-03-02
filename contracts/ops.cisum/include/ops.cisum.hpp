#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/time.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "ops.cisum.db.hpp"

using namespace eosio;
using std::string;
using std::string_view;

namespace flon {

class [[eosio::contract("ops.cisum")]] ops_cisum : public contract {
public:
  using contract::contract;

  ops_cisum(eosio::name receiver, eosio::name code, datastream<const char*> ds)
  : contract(receiver, code, ds),
    _global(get_self(), get_self().value)
  {
    _gstate = _global.exists() ? _global.get() : global_t{};
  }

  ~ops_cisum() {
    if (_save_state) _global.set(_gstate, get_self());
  }

  ACTION init(const name& admin) ;

  ACTION setadmin(const name& admin);

  ACTION setpause(const bool& paused);

  [[eosio::on_notify("cisum.token::transfer")]]
  void ontransfer(const name& from, const name& to, const asset& quantity, const string& memo);

private:
  global_singleton _global;
  global_t         _gstate;
  bool             _save_state = true;

  void require_admin() const;
  static uint64_t to_u64(string_view s, string_view err_title);
};

} // namespace flon
