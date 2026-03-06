#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <flon/consts.hpp>

#include <string>

using namespace eosio;
using std::string;
// ---------- 通用断言码 ----------
#define CHECKC(exp, code, msg) \
   { if (!(exp)) eosio::check(false, string("[[") + std::to_string((int)code) + string("]] ") + msg); }

  enum class err : uint8_t {
  INVALID_FORMAT        = 0,
  INSUFFICIENT_QUANTITY = 3,
  NOT_POSITIVE          = 4,
  SYMBOL_MISMATCH       = 5,
  RECORD_NO_FOUND       = 8,
  ACCOUNT_INVALID       = 11,
  DID_NOT_AUTH          = 16,
  UNDER_MAINTENANCE     = 17,
  EXCEED_LIMIT          = 28,
};

namespace flon {



struct [[eosio::table, eosio::contract("cisum.vault")]] global_t {
  name     admin;
  bool     paused = false;

  EOSLIB_SERIALIZE(global_t, (admin)(paused))
};
using global_singleton = eosio::singleton<"global"_n, global_t>;

} // namespace flon
