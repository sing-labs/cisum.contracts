#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <string>
#include <set>

using std::set;
using std::string;
using namespace eosio;

namespace flon {

// ---------- 通用断言码 ----------
#define CHECKC(exp, code, msg) \
   { if (!(exp)) eosio::check(false, string("[[") + std::to_string((int)code) + string("]] ") + msg); }

enum class err: uint8_t {
   INVALID_FORMAT         = 0,
   TYPE_INVALID           = 1,
   FEE_NOT_FOUND          = 2,
   INSUFFICIENT_QUANTITY  = 3,
   NOT_POSITIVE           = 4,
   SYMBOL_MISMATCH        = 5,
   EXPIRED                = 6,
   PWHASH_INVALID         = 7,
   RECORD_NO_FOUND        = 8,
   NOT_REPEAT_RECEIVE     = 9,
   NOT_EXPIRED            = 10,
   ACCOUNT_INVALID        = 11,
   FEE_NOT_POSITIVE       = 12,
   VAILD_TIME_INVALID     = 13,
   MIN_UNIT_INVALID       = 14,
   REDPACK_EXIST          = 15,
   DID_NOT_AUTH           = 16,
   UNDER_MAINTENANCE      = 17,
   NONE_DELETED           = 19,
   IN_THE_WHITELIST       = 20,
   NON_RENEWAL            = 21,
   AMOUNT_TOO_SMALL       = 22,
   AMOUNT_TOO_LARGE       = 23,
   FEE_NOT_REQUIRED       = 24,
   DID_NOT_SUPPORTED      = 25,
   DID_PACK_SYMBOL_ERR    = 26,
   STATUS_MISMATCH        = 27
};

// ---------- 基本常量 ----------
static constexpr symbol NESTAR_SYMBOL  = symbol(symbol_code("NESTAR"), 1);
static constexpr name   CISUM_CONTRACT = "cisum.token"_n;
static constexpr symbol CISUM_SYMBOL   = symbol(symbol_code("CISUM"), 8);

// 可选：过期逻辑
#ifndef YEAR_SECONDS_FOR_TEST
static constexpr uint64_t YEAR_SECONDS = 365 * 24 * 3600;
#else
#warning "YEAR_SECONDS_FOR_TEST should be used only for test!!!"
static constexpr uint64_t YEAR_SECONDS = YEAR_SECONDS_FOR_TEST;
#endif

#define TBL struct [[eosio::table, eosio::contract("nestar.cisum")]]
#define NTBL(name) struct [[eosio::table(name), eosio::contract("nestar.cisum")]]

// ---------- 全局配置 ----------
NTBL("global") global_t {
   set<name> blacklist;             // 黑名单
   name      issuer;                // NESTAR 发行者
   uint64_t  cisum_to_nestar_rate = 100; // 1 CISUM -> X NESTAR

   EOSLIB_SERIALIZE(global_t, (blacklist)(issuer)(cisum_to_nestar_rate))
};

typedef eosio::singleton< "global"_n, global_t > global_singleton;

// ---------- 余额表 ----------
// scope: owner.value
TBL account_t {
   asset      balance;
   bool       allow_send = false;
   bool       allow_recv = false;
   time_point expired_at;

   uint64_t primary_key() const { return balance.symbol.code().raw(); }

   typedef eosio::multi_index<"accounts"_n, account_t> idx_t;

   EOSLIB_SERIALIZE(account_t, (balance)(allow_send)(allow_recv)(expired_at))
};

// ---------- 统计表 ----------
TBL stats_t {
   asset      supply;
   asset      max_supply;
   time_point created_at;
   bool       paused = false;

   uint64_t primary_key() const { return supply.symbol.code().raw(); }
   uint64_t by_symraw()   const { return supply.symbol.code().raw(); }

   typedef eosio::multi_index
   < "stat"_n, stats_t,
       indexed_by<"symrawidx"_n, const_mem_fun<stats_t, uint64_t, &stats_t::by_symraw>>
   > idx_t;

   EOSLIB_SERIALIZE(stats_t, (supply)(max_supply)(created_at)(paused))
};

// ---------- 艺人白名单 ----------
TBL artist_t {
   name     account;
   bool     enabled = true;

   uint64_t primary_key() const { return account.value; }

   typedef eosio::multi_index<"artists"_n, artist_t> idx_t;

   EOSLIB_SERIALIZE(artist_t, (account)(enabled))
};

// ---------- 兑换记录 ----------
TBL exchange_t {
   uint64_t   id;
   name       user;        // 兑换发起人
   asset      in_cisum;    // 收到 CISUM
   asset      out_nestar;  // 发放 NESTAR
   string     memo;
   time_point created_at;

   uint64_t primary_key() const { return id; }
   uint64_t by_user()     const { return user.value; }

   typedef eosio::multi_index
   < "exchanges"_n, exchange_t,
       indexed_by<"byuser"_n, const_mem_fun<exchange_t, uint64_t, &exchange_t::by_user>>
   > idx_t;

   EOSLIB_SERIALIZE(exchange_t, (id)(user)(in_cisum)(out_nestar)(memo)(created_at))
};

} // namespace flon