#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <string>
#include <set>
#include <flon/nasset.hpp>
#include <flon/consts.hpp>
using std::set;
using std::string;
using namespace eosio;

namespace flon {

// TODO: move to flon base lib
#define CHECK(exp, msg) { if (!(exp)) eosio::check(false, msg); }
#ifndef ASSERT
    #define ASSERT(exp) CHECK(exp, #exp)
#endif
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
   STATUS_MISMATCH        = 27,
   EXCEED_LIMIT           = 28,
   QUANTITY_MISMATCH      = 29,
   INVALID_TIME           = 30
};

// static constexpr uint32_t RATIO_BOOST = 10000;


static const asset            MAX_REWARDS_DEFAULT        = asset(25'5000'0000'0000, CISUM_SYM);

#define TBL struct [[eosio::table, eosio::contract("pop.cisum")]]
#define NTBL(name) struct [[eosio::table(name), eosio::contract("pop.cisum")]]


NTBL("global") global_t {
   name           oracle_contract      = CISUM_BANK;
   name           reward_contract      = CISUM_BANK;
   asset          max_rewards          = MAX_REWARDS_DEFAULT;
   asset          available_rewards    = MAX_REWARDS_DEFAULT;
   asset          issued_rewards       = asset(0, CISUM_SYM);
   asset          receivable           = asset(0, USDT_SYM);
   std::set<name> executors;
   EOSLIB_SERIALIZE(global_t, (oracle_contract)(reward_contract)(max_rewards)(available_rewards)(issued_rewards)(receivable)(executors))
};

typedef eosio::singleton< "global"_n, global_t > global_singleton;


TBL receivable_t {
   asset       amount;         // 当前应收金额
   time_point  updated_at;     // 最近更新时间

   uint64_t primary_key() const { return 0; } // 单行表

   EOSLIB_SERIALIZE(receivable_t, (amount)(updated_at))
};
using receivable_singleton = eosio::singleton<"receivable"_n, receivable_t>;


TBL badge_rule_t {
   uint64_t       id;
   asset          threshold;        // 达到多少 consumed.amount 送此勋章
   nsymbol        symbol;        // 勋章编码
   bool           enabled = true;
   time_point     created_at;

   uint64_t  primary_key()    const { return id; }
   uint64_t  by_threshold()      const { return threshold.amount; }
   uint64_t  by_symbol()      const { return symbol.raw(); }

   typedef eosio::multi_index<
     "badgerules"_n, badge_rule_t,
     indexed_by<"bythreshold"_n, const_mem_fun<badge_rule_t,uint64_t,&badge_rule_t::by_threshold>>,
     indexed_by<"bysymbol"_n, const_mem_fun<badge_rule_t,uint64_t,&badge_rule_t::by_symbol>>
   > idx_t;
    EOSLIB_SERIALIZE(badge_rule_t, (id)(threshold)(symbol)(enabled)(created_at))
};



} // namespace flon