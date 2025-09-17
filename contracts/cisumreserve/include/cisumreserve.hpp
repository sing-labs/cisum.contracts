#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <string>


#include "cisumreserve.db.hpp"
#include <flon.swap/flon.swap.db.hpp>
#include <flon/consts.hpp>

namespace flon {

using namespace eosio;
using std::string;

/**
 * cisumreserve 合约：接收 USDT -> 按 flon.swap 价格折算 -> 发 CISUM 给付款人，并记录订单
 *
 * 配置存于 global：
 *   - admin        : 管理员
 *   - fee_bps      : 手续费（基点，1 = 0.01%）
 *   - last_order_id: 自增订单号
 *
 * 监听入金：使用通配通知 "*::transfer"，在实现中用 get_first_receiver() 过滤 usdt_bank。
 */
class [[eosio::contract("cisumreserve")]] cisumreserve : public contract {
public:
    using contract::contract;

    cisumreserve(eosio::name receiver, eosio::name code, datastream<const char*> ds)
    : contract(receiver, code, ds),
      _global(get_self(), get_self().value)
    {
      _gstate = _global.exists() ? _global.get() : global_t{};
    }

    ~cisumreserve() { _global.set(_gstate, get_self()); }

    ACTION init(const name&    admin,const uint16_t& fee_bps);

    ACTION setfee(const name& submitter, const uint16_t& fee_bps);

    ACTION setadmin(const name& submitter, const name& new_admin);

    // ========== 充值监听（USDT 入金）==========
    // 采用通配通知，具体在 .cpp 中判断 first_receiver == g.usdt_bank
    [[eosio::on_notify("flon.mtoken::transfer")]]
    void on_usdt_transfer(const name& from,
                     const name& to,
                     const asset& quantity,
                     const string& memo);

    [[eosio::on_notify("cisum.token::transfer")]]
    void on_cisum_transfer(const name& from,
                     const name& to,
                     const asset& quantity,
                     const string& memo);

private:
  global_singleton _global;
  global_t         _gstate;

private:
  // 小写化 symbol_code -> std::string
  static std::string to_lower(const symbol_code& sc) {
    std::string s = sc.to_string();
    for (char& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
  }

  // 10^p（0<=p<=18）
  static int64_t pow10(int p) {
    int64_t v = 1;
    while (p-- > 0) v *= 10;
    return v;
  }

  // 从 flon.swap 读取池子并返回按 left 精度放大的 "quote per left"
  inline asset get_price_from_swap_as_asset(const symbol& left_sym,
                                            const symbol& right_sym)
  {
    const name swap_ctr = SWAP_CONTRACT;
    const name tpcode{ to_lower(left_sym.code()) + "." + to_lower(right_sym.code()) };

    flon::market_t::idx_t markets(swap_ctr, swap_ctr.value);
    auto it = markets.find(tpcode.value);
    check(it != markets.end(), ("market not found: " + tpcode.to_string()).c_str());

    const asset& L = it->left_pool_quant.quantity;   // left 池量
    const asset& R = it->right_pool_quant.quantity;  // right 池量
    check(L.symbol == left_sym && R.symbol == right_sym, "symbol mismatch in market");
    check(L.amount > 0 && R.amount > 0, "empty pool");

    const int64_t scale = pow10(L.symbol.precision());
    __int128 num = (__int128)R.amount * (__int128)scale;
    int64_t price_amount = (int64_t)(num / (__int128)L.amount); // floor

    return asset{ price_amount, right_sym };
  }

  // usdt(asset,6) -> cisum(asset,8)，按 swap 价格换算
  asset usdt_to_cisum(const asset& usdt) {
    check(usdt.symbol == USDT_SYM, "usdt_to_cisum: usdt symbol mismatch");
    asset price = this->get_price_from_swap_as_asset(CISUM_SYM, USDT_SYM);
    check(price.amount > 0, "invalid price");

    // cisum_amount = usdt.amount * 10^8 / price.amount
    const int64_t p10C = pow10(CISUM_SYM.precision());
    __int128 num = (__int128)usdt.amount * (__int128)p10C;
    int64_t cisum_units = (int64_t)(num / (__int128)price.amount);
    return asset{ cisum_units, CISUM_SYM };
  }
};

} // namespace flon