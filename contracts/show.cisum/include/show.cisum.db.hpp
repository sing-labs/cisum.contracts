#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <eosio/system.hpp>
#include <set>
#include <string>

#include <cvticket.nft.hpp>

namespace flon {

using namespace eosio;
using std::string;
using std::set;

namespace ShowStatus {
   static constexpr name onshelf = "onshelf"_n;   // 上架/售卖中
   static constexpr name offsale = "offsale"_n;   // 下架/暂停售卖
   static constexpr name closed  = "closed"_n;    // 结束/关闭（不可再改）
}

namespace TicketStatus {
   static constexpr name running = "running"_n;   // 可售/可发放
   static constexpr name paused  = "paused"_n;    // 暂停
   static constexpr name closed  = "closed"_n;    // 已关闭
}

#define TBL        struct [[eosio::table, eosio::contract("show.cisum")]]
#define NTBL(name) struct [[eosio::table(name), eosio::contract("show.cisum")]]

NTBL("global") global_t {
  name        admin;          // 超管
  set<name>   show_admin;     // 演出管理员白名单
  set<name>   platform_admin; // 平台管理员白名单
  name        nft_bank = "cvticket.nft"_n;       // 票 NFT 合约账户

  EOSLIB_SERIALIZE(global_t, (admin)(show_admin)(platform_admin)(nft_bank))
};
using global_singleton = eosio::singleton<"global"_n, global_t>;

// 演出主表（scope: self）
TBL show_t {
   uint64_t   show_id;                              // 主键
   name       category;                             // 类别（自定义：concert/drama/exhibit...）
   bool       ticket_transferable = false;          // 门票是否可转让
   bool       ticket_refundable   = false;          // 门票是否可退票
   set<name>  ticket_check_admins;                  // 核销员
   name       status = ShowStatus::onshelf;
   time_point show_started_at;                      // 演出开始
   time_point show_ended_at;                        // 演出结束
   name       show_name;                            // 演唱会名称
   name       show_address;                         // 演唱会地址
   time_point created_at;
   time_point updated_at;

   uint64_t primary_key() const { return show_id; }

   typedef eosio::multi_index<"shows"_n, show_t> showidx;

   EOSLIB_SERIALIZE(show_t,
      (show_id)(category)
      (ticket_transferable)(ticket_refundable)
      (status)
      (show_started_at)(show_ended_at)
      (show_name)(show_address)(created_at)(updated_at)
            (ticket_check_admins)
   )
};

// 票档表（scope: show_id）
TBL ticket_t {
   uint64_t   ticket_id;                // 对应票 NFT 的 nsymbol.raw()
   uint64_t   prerequisite_ticket_id;   // 前置/父票（nsymbol.raw()，无则 0）
   string     ticket_type;              // 普通/合影/晚宴…
   asset      price;                    // 价格（例："100.00 USD"）
   uint32_t   total_count;              // 总量（<= NFT 总发行量）
   uint32_t   sold_count;               // 已售/已分配（含待发）
   uint32_t   stock_count;              // 剩余库存（= total_count - sold_count）
   uint32_t   issued_count;             // 已实际发放的 NFT 数
   time_point sale_started_at;          // 售票开始
   time_point sale_ended_at;            // 售票结束
   name       status;

   time_point created_at;
   time_point updated_at;

   uint64_t primary_key() const { return ticket_id; }

   typedef eosio::multi_index<"tickets"_n, ticket_t> ticketidx;

   EOSLIB_SERIALIZE(ticket_t,
      (ticket_id)(prerequisite_ticket_id)
      (ticket_type)(price)
      (total_count)(sold_count)(stock_count)(issued_count)
      (sale_started_at)(sale_ended_at)(status)
      (created_at)(updated_at)
   )
};


} // namespace flon