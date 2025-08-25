#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <string>
#include <cctype>

#include "poh.cisum.db.hpp"
#include "flon.swap/flon.swap.db.hpp"

using namespace eosio;
using std::string;

namespace flon {

class [[eosio::contract("poh.cisum")]] poh_cisum : public contract {
public:
  using contract::contract;

  poh_cisum(eosio::name receiver, eosio::name code, datastream<const char*> ds)
  : contract(receiver, code, ds),
    _global(get_self(), get_self().value)
  {
    _gstate = _global.exists() ? _global.get() : global_t{};
  }

  ~poh_cisum() { _global.set(_gstate, get_self()); }

  [[eosio::action]] void init(name platform, name registrar);
  [[eosio::action]] void setplatform(name platform);
  [[eosio::action]] void setregistrar(name registrar);
  [[eosio::action]] void setrewards(const asset& available);

  [[eosio::action]] void registreward(name user, string memo);

  using registreward_action = eosio::action_wrapper<"registreward"_n, &poh_cisum::registreward>;

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

  // usd(asset,6) -> cisum(asset,8)，按 swap 价格换算
  asset usd_to_cisum(const asset& usd) {
    check(usd.symbol == USDT_SYM, "usd_to_cisum: usd symbol mismatch");
    asset price = this->get_price_from_swap_as_asset(CISUM_SYM, USDT_SYM);
    check(price.amount > 0, "invalid price");

    // cisum_amount = usd.amount * 10^8 / price.amount
    const int64_t p10C = pow10(CISUM_SYM.precision());
    __int128 num = (__int128)usd.amount * (__int128)p10C;
    int64_t cisum_units = (int64_t)(num / (__int128)price.amount);
    return asset{ cisum_units, CISUM_SYM };
  }

  // 从 cisum.token 给平台打款（需要 poh.cisum@active add-code）
  void pay_reward_to_platform(const asset& reward, const string& memo) {
    action(
      permission_level{ get_self(), "active"_n },
      REWARD_BANK,
      "transfer"_n,
      std::make_tuple(get_self(), _gstate.platform_acct, reward, memo)
    ).send();
  }
};

} // namespace flon