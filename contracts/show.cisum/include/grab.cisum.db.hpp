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

static constexpr uint32_t RATIO_BOOST = 10000;
#define NTBL(name) struct [[eosio::table(name), eosio::contract("grab.cisum")]]
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
   nasset         grabbed_tickets;
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
     (total_tickets)(available_tickets)(grabbed_tickets)(total_grabs)
     (created_at)(updated_at)
   )
};

} // namespace flon