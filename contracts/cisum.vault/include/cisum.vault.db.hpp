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

struct [[eosio::table, eosio::contract("cisum.vault")]] paytype_t {
  uint64_t id;
  name pay_type;
  string desc;
  time_point created_at;

  uint64_t primary_key() const { return id; }
  uint64_t by_type() const { return pay_type.value; }

  EOSLIB_SERIALIZE(paytype_t, (id)(pay_type)(desc)(created_at))
};
using paytypes_idx = eosio::multi_index<"paytypes"_n, paytype_t,
  indexed_by<"bytype"_n, const_mem_fun<paytype_t, uint64_t, &paytype_t::by_type>>>;

} // namespace flon
