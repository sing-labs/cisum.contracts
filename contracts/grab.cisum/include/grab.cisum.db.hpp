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

static constexpr uint32_t RATIO_BOOST = 10000;

static constexpr symbol_code POINT_SYMBOL_CODE  = symbol_code("NESTAR");
static constexpr symbol POINT_SYMBOL            = symbol(POINT_SYMBOL_CODE, 4);
static constexpr name   POINT_CONTRACT_DEFAULT  = "nestar.token"_n;
static constexpr name   TICKET_CONTRACT_DEFAULT = "cvticket.nft"_n;

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
   nsymbol        show_id;
   nsymbol        ticket_id;
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

   uint64_t primary_key() const { return id; }

   typedef eosio::multi_index<"rushsales"_n, rush_sale> idx_t;

   EOSLIB_SERIALIZE(rush_sale, (id)(show_id)(ticket_id)(started_at)(ended_at)(price)(max_grabs_per_user)(win_ratio)(total_tickets)(available_tickets)(sold_tickets)(total_grabs)(created_at)(updated_at))
};

// scope: rush_sale_id
NTBL("users") user_t {
   eosio::name    account;
   uint32_t       grabs;
   nasset         tickets;

   uint64_t primary_key() const { return account.value; }

   typedef eosio::multi_index<"users"_n, user_t> idx_t;

   EOSLIB_SERIALIZE(user_t, (account)(grabs)(tickets))
};

} // namespace flon