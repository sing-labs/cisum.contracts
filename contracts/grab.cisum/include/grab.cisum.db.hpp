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

//static constexpr uint32_t RATIO_BOOST = 10000;

static constexpr uint32_t RATIO_BASE = 10000;   // 100.00%

#define TBL struct [[eosio::table, eosio::contract("grab.cisum")]]
#define NTBL(name) struct [[eosio::table(name), eosio::contract("grab.cisum")]]


NTBL("grabglobal") global_t {
   uint64_t       last_rush_sale_id;
   eosio::name    admin;
   eosio::name    point_contract    = NESTAR_CONTRACT;
   eosio::name    ticket_contract   = CVTICKET_CONTRACT;
   set<name>      oracles;

   EOSLIB_SERIALIZE(global_t, (last_rush_sale_id)(admin)(point_contract)(ticket_contract)(oracles))
};

typedef eosio::singleton< "grabglobal"_n, global_t > global_singleton;

// scope: self
NTBL("rushsales") rush_sale {
   uint64_t       id; // auto increment, PK
   uint64_t       show_id;
   uint64_t       ticket_id;
   time_point     started_at;
   time_point     ended_at;
   asset          price;
   uint32_t       max_grabs_per_user;
   uint32_t       win_ratio;              // boost 10000, <= 10000
   nasset         total_tickets;
   nasset         available_tickets;
   nasset         sold_tickets;
   uint32_t       total_grabs;
   time_point     created_at;
   time_point     updated_at;

   // 主键
   uint64_t primary_key() const { return id; }

   // 二级索引键函数
   uint64_t byticket() const { return ticket_id; }
   uint64_t byshow()   const { return show_id;   }

   typedef eosio::multi_index<
      "rushsales"_n,
      rush_sale,
      indexed_by<"byticket"_n, const_mem_fun<rush_sale, uint64_t, &rush_sale::byticket>>,
      indexed_by<"byshow"_n,   const_mem_fun<rush_sale, uint64_t, &rush_sale::byshow>>
   >idx_t;

   EOSLIB_SERIALIZE(rush_sale,
     (id)(show_id)(ticket_id)(started_at)(ended_at)(price)
     (max_grabs_per_user)(win_ratio)
     (total_tickets)(available_tickets)(sold_tickets)(total_grabs)
     (created_at)(updated_at)
   )
};

// scope: rush_sale_id
NTBL("orders") order_t {
   uint64_t       id;          // 自增主键
   std::string    grab_id;     // md5 hex，业务唯一
   eosio::name    account;     // 中奖账号
   uint32_t       grabs;       // 固定写 1（表示一次抽奖）
   nasset         tickets;     // 固定 1 * ticket_symbol
   time_point     created_at;  // 中奖时间

   bool is_win() const { return tickets.amount > 0; }
   uint128_t by_userwin() const {
      return ( (uint128_t)account.value << 1 ) | (is_win() ? 1 : 0);
   }
   // —— 主键：用自增 id
   uint64_t primary_key() const { return id; }
   uint64_t byaccount() const { return account.value; }
   // —— 二级索引：grab_id 唯一
   checksum256 by_grabid() const {
       return sha256(grab_id.data(), grab_id.size());
   }

   typedef eosio::multi_index<
     "orders"_n, order_t,
     indexed_by<"bygrabid"_n, const_mem_fun<order_t, checksum256, &order_t::by_grabid>>,
     indexed_by<"byaccount"_n, const_mem_fun<order_t, uint64_t,   &order_t::byaccount>>,
     indexed_by<"byuserwin"_n, const_mem_fun<order_t, uint128_t, &order_t::by_userwin>>
   > idx_t;

   EOSLIB_SERIALIZE(order_t, (id)(grab_id)(account)(grabs)(tickets)(created_at))
};

// scope = rush_sale_id
NTBL("grabstats") grab_stat_t {
    eosio::name account;     // 主键
    uint32_t    grabs = 0;   // 已参与次数
    time_point  updated_at;

    uint64_t primary_key() const { return account.value; }
    using idx_t = eosio::multi_index<"grabstats"_n, grab_stat_t>;
      EOSLIB_SERIALIZE(grab_stat_t, (account)(grabs)(updated_at))
};


// scope: self
NTBL("allowtokens") allowed_token_t {
    uint64_t    id;          // 自增主键
    symbol      sym;         // 币种(含精度)，例如 4,NESTAR / 8,CISUM
    name        bank;        // 发行/转账合约账号，例如 nestar.token / cisum.token
    time_point  created_at;
    time_point  updated_at;

    uint64_t primary_key() const { return id; }
    uint64_t bycode()      const { return sym.code().raw(); } // 便于按币种代码查找（忽略精度）
    uint128_t bysymbol()   const {
        return (uint128_t(sym.code().raw()) << 64) | (uint128_t) sym.precision();
    }

    typedef eosio::multi_index<
        "allowtokens"_n, allowed_token_t,
        indexed_by<"bycode"_n,   const_mem_fun<allowed_token_t, uint64_t,  &allowed_token_t::bycode>>,
        indexed_by<"bysymbol"_n, const_mem_fun<allowed_token_t, uint128_t, &allowed_token_t::bysymbol>>
    > idx_t;

    EOSLIB_SERIALIZE(allowed_token_t, (id)(sym)(bank)(created_at)(updated_at))
};


} // namespace flon