#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <string>
#include <set>
#include <vector>
#include <flon/nasset.hpp>

using std::set;
using std::string;
using std::vector;
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
static constexpr symbol NESTAR_SYMBOL  = symbol(symbol_code("NESTAR"), 4);


#define TBL struct [[eosio::table, eosio::contract("nestar.token")]]
#define NTBL(name) struct [[eosio::table(name), eosio::contract("nestar.token")]]

// ---------- 全局配置 ----------
NTBL("global") global_t {
   name      issuer;                    // NESTAR 发行者
   name      admin;                     // 管理员账户
   name      artists_contract;          // 艺人注册合约
   name      badgestore_contract ;      // 勋章 发放 合约
   EOSLIB_SERIALIZE(global_t, (issuer)(admin)(artists_contract)(badgestore_contract))
};

typedef eosio::singleton< "global"_n, global_t > global_singleton;

// ---------- 余额表 ----------
// scope: owner.value
TBL account_t {
   asset      balance;        // 当前可用余额
   asset      consumed;       // 已累计消耗的余额
   time_point created_at;
   time_point updated_at;
   time_point expired_at;

   uint64_t primary_key() const { return balance.symbol.code().raw(); }

   typedef eosio::multi_index<"accounts"_n, account_t> idx_t;

   EOSLIB_SERIALIZE(account_t, (balance)(consumed)(created_at)(updated_at)(expired_at))
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

// ---------- 功能白名单（允许被转账/收款的账户或合约） ----------
TBL transfer_whitelist_t {
   name account;          // 被允许的账户/合约
   bool enabled = true;   // 开关

   uint64_t primary_key() const { return account.value; }

   typedef eosio::multi_index<"transferwlst"_n, transfer_whitelist_t> idx_t;

   EOSLIB_SERIALIZE(transfer_whitelist_t, (account)(enabled))
};

TBL redeem_whitelist_t {
   name account;          // 被允许的账户/合约
   bool enabled = true;   // 开关

   uint64_t primary_key() const { return account.value; }

   typedef eosio::multi_index<"redeemwlst"_n, redeem_whitelist_t> idx_t;

   EOSLIB_SERIALIZE(redeem_whitelist_t, (account)(enabled))
};



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