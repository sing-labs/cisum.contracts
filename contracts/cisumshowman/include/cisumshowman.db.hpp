#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <flon/nasset.hpp>
#include <eosio/singleton.hpp>
#include <vector>
#include <string>

using namespace eosio;
using std::string;
using std::vector;

namespace flon {

static constexpr name SHOW_CONTRACT             = "show24.cisum"_n;
static constexpr name GRAB_CONTRACT             = "grab23.cisum"_n;
static constexpr symbol NESTAR_SYM              = symbol(symbol_code("NESTAR"), 4);
static constexpr symbol CISUM_SYM               = symbol(symbol_code("CISUM"), 8);
static constexpr symbol USDT_SYM                 = symbol(symbol_code("USDT"), 6);

struct [[eosio::table, eosio::contract("cisumshowman")]] global_t {
    name admin;
    EOSLIB_SERIALIZE(global_t, (admin))
};
using global_singleton = eosio::singleton<"global"_n, global_t>;


// —— 票档入参（非表）——
struct ticket_info {
    uint64_t   ticket_id;                 // 与 nsymbol(raw) 对应
    string     token_uri;                 // NFT 元数据 URI
    string     ticket_type;               // 票种类型
    asset      price;                     // 原来币种的价格（NESTAR是免费票）
    asset      price_usdt;                 // 转化的usd价格
    uint64_t   total_count;               // 发行量
    uint64_t   prerequisite_ticket_id = 0;// 前置票（无则 0）
    time_point sale_started_at;           // 售票开始
    time_point sale_ended_at;             // 售票结束
    uint32_t   win_ratio;                 // 抢票中奖率（万分比，0-10000）
    uint32_t   max_grabs_per_user;        // 每人最大抢票数

    EOSLIB_SERIALIZE(ticket_info,
        (ticket_id)(token_uri)(ticket_type)(price)(price_usdt)
        (total_count)(prerequisite_ticket_id)
        (sale_started_at)(sale_ended_at)(win_ratio)(max_grabs_per_user)
    )
};

// —— 演出入参（非表）——
struct show_info {
    uint64_t   show_id;
    name       category;
    bool       ticket_transferable = false;
    bool       ticket_refundable   = false;
    time_point show_started_at;
    time_point show_ended_at;
    string     show_name;
    string     show_address;

    EOSLIB_SERIALIZE(show_info,
        (show_id)(category)
        (ticket_transferable)(ticket_refundable)
        (show_started_at)(show_ended_at)
        (show_name)(show_address)
    )
};

} // namespace flon