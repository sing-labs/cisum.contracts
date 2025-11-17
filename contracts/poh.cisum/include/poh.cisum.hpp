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

  ACTION init(name platform, name registrar, asset max_issued) ;

  ACTION setmaxissued(asset max_issued);

  ACTION setplatform(name platform);
  ACTION setregistrar(name registrar);

  ACTION registreward(const name& submitter,
                              const name& inviter,    // 可为空：inviter.value==0 表示无邀请人
                              const name& invitee     // 被邀请人（拿主奖励）
                                   );

  [[eosio::on_notify("*::transfer")]]
  void on_transfer(const name& from, const name& to,const asset& quantity, const string& memo);

  ACTION setfundinfo( const name& inviter,
                        const symbol sym,
                        const name& contract,
                        const string& reward_title,
                        const std::optional<uint32_t>& start_ts,
                        const std::optional<uint32_t>& end_ts);

  ACTION redeemfund(const name& oper, const name& inviter);

  ACTION notifyreward(const name&  from,
                                const name&       to,
                                const asset&      award_amount,
                                const string&     memo,
                                const name&       reward_type,
                                const string&     reward_ref_id,
                                const uint64_t&   created_at);

  using registreward_action = eosio::action_wrapper<"registreward"_n, &poh_cisum::registreward>;
  using notifyreward_action  = eosio::action_wrapper<"notifyreward"_n, &poh_cisum::notifyreward>;

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
  inline asset get_price_from_swap(const symbol& left_sym,
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

  // usdt(asset,6) -> sing(asset,8)，按 swap 价格换算
  asset exchange_asset(const asset& usdt) {
    check(usdt.symbol == USDT_SYM, "exchange_asset: usdt symbol mismatch");
    asset price = this->get_price_from_swap(SING_SYM, USDT_SYM);
    check(price.amount > 0, "invalid price");

    //sing_amount = usdt.amount * 10^8 / price.amount
    const int64_t p10C = pow10(SING_SYM.precision());
    __int128 num = (__int128)usdt.amount * (__int128)p10C;
    int64_t token_units = (int64_t)(num / (__int128)price.amount);
    return asset{ token_units, SING_SYM };
  }

 void _reward_invitee(const name& invitee) ;
 void _reward_inviter(const name& inviter, const name& invitee);
 void _send_inviter_fund(const name& inviter, const name& invitee);
 void _merge_fund_balance_s(const name& inviter, const extended_symbol& ext_symb, const fund_balance_s& fund_balance);


};

} // namespace flon