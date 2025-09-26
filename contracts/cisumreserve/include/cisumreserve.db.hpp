#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <string>

namespace flon {

using namespace eosio;
using std::string;

// 价格缩放：sing_per_1USDT * 1e6（避免浮点）
static constexpr uint32_t PRICE_PPM_SCALE = 1'000'000;

// ============ global ============
struct [[eosio::table("global"), eosio::contract("cisumreserve")]] global_t {
    name    admin;          // 管理员
    uint16_t fee_bps = 0;   // 手续费（基点，1 = 0.01%）
    uint64_t last_order_id = 0; // 订单自增 id

    EOSLIB_SERIALIZE( global_t,(admin)(fee_bps)(last_order_id))
};
using global_singleton = eosio::singleton<"global"_n, global_t>;

// ============ orders：兑换记录 ============
struct [[eosio::table("orders"), eosio::contract("cisumreserve")]] order_t {
    uint64_t    id;           // 自增主键
    name        user;         // 充值人（USDT 付款方）
    asset       usdt_in;      // 收到的 USDT
    asset       token_out;    // 实际发放的资产（可能是 SING / CISUM / 其他）（扣除手续费后）
    uint64_t    rate_ppm;     // 价格：每 1 USDT 可得多少 资产（*1e6）
    uint16_t    fee_bps;      // 成交时使用的手续费 bps
    time_point  created_at;   // 成交时间
    string      memo;         // 备注（原始 memo 或处理说明）

    uint64_t primary_key() const { return id; }
    uint64_t byuser()      const { return user.value; }
    uint64_t bytime()      const { return (uint64_t)created_at.sec_since_epoch(); }

    EOSLIB_SERIALIZE(order_t,
        (id)(user)(usdt_in)(token_out)(rate_ppm)(fee_bps)(created_at)(memo)
    )
};
using orders_idx = eosio::multi_index<
    "orders"_n, order_t,
    indexed_by<"byuser"_n, const_mem_fun<order_t, uint64_t, &order_t::byuser>>,
    indexed_by<"bytime"_n, const_mem_fun<order_t, uint64_t, &order_t::bytime>>
>;

} // namespace flon