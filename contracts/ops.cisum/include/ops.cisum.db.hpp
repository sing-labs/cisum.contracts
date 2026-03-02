#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/time.hpp>
#include <flon/consts.hpp>

#include <set>
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



struct [[eosio::table, eosio::contract("ops.cisum")]] global_t {
  name     admin;
  bool     paused          = false;
  uint64_t last_pay_id     = 0;

  EOSLIB_SERIALIZE(global_t, (admin)(paused)(last_pay_id))
};
using global_singleton = eosio::singleton<"global"_n, global_t>;

struct [[eosio::table, eosio::contract("ops.cisum")]] payment_t {
  uint64_t   id;
  name       payer;
  name       kind; // exchange / livepay
  asset      quantity;
  string     memo;
  time_point created_at;

  uint64_t primary_key() const { return id; }
  uint64_t by_payer() const { return payer.value; }
  uint64_t by_kind() const { return kind.value; }

  typedef eosio::multi_index<
      "payments"_n, payment_t,
      indexed_by<"bypayer"_n, const_mem_fun<payment_t, uint64_t, &payment_t::by_payer>>,
      indexed_by<"bykind"_n, const_mem_fun<payment_t, uint64_t, &payment_t::by_kind>>
  > idx_t;

  EOSLIB_SERIALIZE(payment_t, (id)(payer)(kind)(quantity)(memo)(created_at))
};

} // namespace flon
