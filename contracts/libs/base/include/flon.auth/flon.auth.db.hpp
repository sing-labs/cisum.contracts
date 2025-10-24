#pragma once
#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <eosio/crypto.hpp>   // <- 需要 sha256 / checksum256
#include <set>
#include <string>
#include <vector>

using namespace eosio;
using std::string;

namespace flon {


struct [[eosio::table, eosio::contract("flon.auth")]] auth_global_t {
  name           admin;
  std::set<name> allowlist;
  uint64_t       last_role_id=0;
  uint64_t       last_userrole_id=0;
  uint64_t       last_roleperm_id=0;
  EOSLIB_SERIALIZE(auth_global_t, (admin)(allowlist)(last_role_id)(last_userrole_id)(last_roleperm_id))
};
using flonauth_global = eosio::singleton<"global"_n, auth_global_t>;


} // namespace flon