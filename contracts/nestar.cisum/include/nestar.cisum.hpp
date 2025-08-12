#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <string>

// 引入表、全局配置、常量、CHECKC/err
#include "nestar.cisum.db.hpp"

using std::string;
using namespace eosio;

namespace flon {

class [[eosio::contract("nestar.cisum")]] nestar : public contract {
public:
  using contract::contract;

  nestar(eosio::name receiver, eosio::name code, datastream<const char*> ds)
  : contract(receiver, code, ds),
    _global(get_self(), get_self().value)
  {
    _gstate = _global.exists() ? _global.get() : global_t{};
  }

  ~nestar() { _global.set(_gstate, get_self()); }

  // -------- Actions --------
  [[eosio::action]]
  void create(const name& issuer, const asset& maximum_supply);

  [[eosio::action]]
  void issue(const name& to, const asset& quantity, const std::string& memo);

  [[eosio::action]]
  void retire(const asset& quantity, const string& memo);

  // 1) issuer -> any
  // 2) user   -> artist (whitelist)
  [[eosio::action]]
  void transfer(const name& from, const name& to, const asset& quantity, const std::string& memo);

  [[eosio::action]]
  void open(const name& owner, const symbol& sym, const name& ram_payer);

  [[eosio::action]]
  void setartist(const name& artist, bool enabled);

  [[eosio::action]]
  void setissuer(const name& issuer);

  [[eosio::action]]
  void setrate(const uint64_t& rate);

  [[eosio::action]]
  void setblacklist(const name& account, const bool& banned);

  [[eosio::action]]
  void setacctperms(const name& issuer, const name& to, const symbol& sym, const bool& allowsend, const bool& allowrecv);

  // CISUM 充值兑换
  [[eosio::on_notify("cisum.token::transfer")]]
  void on_transfer(const name& from, const name& to, const asset& quantity, const std::string& memo);

  // -------- Inline wrappers --------
  using create_action        = eosio::action_wrapper<"create"_n,        &nestar::create>;
  using issue_action         = eosio::action_wrapper<"issue"_n,         &nestar::issue>;
  using transfer_action      = eosio::action_wrapper<"transfer"_n,      &nestar::transfer>;
  using open_action          = eosio::action_wrapper<"open"_n,          &nestar::open>;
  using setartist_action     = eosio::action_wrapper<"setartist"_n,     &nestar::setartist>;
  using setacctperms_action  = eosio::action_wrapper<"setacctperms"_n,  &nestar::setacctperms>;
  using setissuer_action     = eosio::action_wrapper<"setissuer"_n,     &nestar::setissuer>;
  using setrate_action       = eosio::action_wrapper<"setrate"_n,       &nestar::setrate>;
  using setblacklist_action  = eosio::action_wrapper<"setblacklist"_n,  &nestar::setblacklist>;

  // -------- Helpers (static) --------
  static asset get_supply(const name& token_contract_account, const symbol_code& sym_code) {
    stats_t::idx_t statstable(token_contract_account, sym_code.raw());
    const auto& st = statstable.get(sym_code.raw());
    return st.supply;
  }

  static asset get_balance(const name& token_contract_account, const name& owner, const symbol_code& sym_code) {
    account_t::idx_t accountstable(token_contract_account, owner.value);
    const auto& ac = accountstable.get(sym_code.raw());
    return ac.balance;
  }

  static bool account_exist(const name& token_contract_account, const name& owner, const symbol_code& sym_code) {
    account_t::idx_t accountstable(token_contract_account, owner.value);
    return accountstable.find(sym_code.raw()) != accountstable.end();
  }

private:
  // 全局
  global_singleton _global;
  global_t         _gstate;

  // -------- internal methods --------
  void sub_balance(const name& owner, const asset& value);
  void add_balance(const name& owner, const asset& value, const name& ram_payer);

  inline void require_issuer(const name& actor) const {
    CHECKC(_gstate.issuer.value != 0, err::RECORD_NO_FOUND, "global issuer not set");
    CHECKC(actor == _gstate.issuer,   err::DID_NOT_AUTH,     "issuer only");
  }

  bool is_artist_enabled(const name& acc) const {
    artist_t::idx_t atbl(get_self(), get_self().value);
    auto it = atbl.find(acc.value);
    return (it != atbl.end() && it->enabled);
  }
};

} // namespace flon