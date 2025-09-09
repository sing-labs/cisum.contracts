#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <string>
#include <set>
#include <flon/nasset.hpp>

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

static constexpr symbol_code POINT_SYMBOL_CODE  = symbol_code("NESTAR");
static constexpr symbol POINT_SYMBOL            = symbol(POINT_SYMBOL_CODE, 4);
static constexpr name   POINT_CONTRACT_DEFAULT  = "nestar.token"_n;
static constexpr name   TICKET_CONTRACT_DEFAULT = "cvticket.nft"_n;

static constexpr name SHOW_CONTRACT = "show23.cisum"_n;
static constexpr name OPS_CONTRACT = "ops15.cisum"_n;   // ops  操作调度中心合约账户

#define TBL struct [[eosio::table, eosio::contract("grab.cisum")]]
#define NTBL(name) struct [[eosio::table(name), eosio::contract("grab.cisum")]]


NTBL("global") global_t {
   uint64_t       last_rush_sale_id;
   eosio::name    admin;
   eosio::name    point_contract = POINT_CONTRACT_DEFAULT;
   eosio::name    ticket_contract = TICKET_CONTRACT_DEFAULT;

   EOSLIB_SERIALIZE(global_t, (last_rush_sale_id)(admin)(point_contract)(ticket_contract))
};

typedef eosio::singleton< "global"_n, global_t > global_singleton;

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

   // —— 主键：用自增 id
   uint64_t primary_key() const { return id; }

   // —— 二级索引：grab_id 唯一
   checksum256 by_grabid() const {
       return sha256(grab_id.data(), grab_id.size());
   }

   typedef eosio::multi_index<
     "orders"_n, order_t,
     indexed_by<"bygrabid"_n, const_mem_fun<order_t, checksum256, &order_t::by_grabid>>
   > idx_t;

   EOSLIB_SERIALIZE(order_t, (id)(grab_id)(account)(grabs)(tickets)(created_at))
};

} // namespace flon