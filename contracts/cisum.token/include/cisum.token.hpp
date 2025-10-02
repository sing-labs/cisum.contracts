#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <string>
#include "cisum.token.db.hpp"



using std::string;
using namespace eosio;
using std::vector;
namespace flon {

class [[eosio::contract("cisum.token")]] cisum_token : public contract {
public:
  using contract::contract;

  cisum_token(eosio::name receiver, eosio::name code, datastream<const char*> ds)
  : contract(receiver, code, ds),
    _global(get_self(), get_self().value)
  {
    _gstate = _global.exists() ? _global.get() : global_t{};
  }

  ~cisum_token() { _global.set(_gstate, get_self()); }

  [[eosio::action]]
  void init(name issuer, name admin, name badgestore_contract);

  [[eosio::action]] void addwhitelist(const name& account);
  [[eosio::action]] void delwhitelist(const name& account);

  [[eosio::action]] void addconsumewl(const name& account);
  [[eosio::action]] void delconsumewl(const name& account);

  [[eosio::action]]
  void create(const name& issuer, const asset& maximum_supply);

  [[eosio::action]]
  void issue(const name& to, const asset& quantity, const std::string& memo);

  [[eosio::action]]
  void retire(const asset& quantity, const string& memo);

  // 允许：
  // 1) 发行者 -> 任意账户/合约
  // 2) 普通用户 -> 功能白名单账户/合约（如艺人账户、业务合约等）
  // 同时会在“用户支出场景”里累计 consumed 并触发自动勋章发放
  [[eosio::action]]
  void transfer(const name& from, const name& to, const asset& quantity, const std::string& memo);


  [[eosio::action]]
  void open(const name& owner, const symbol& sym, const name& ram_payer);

  [[eosio::action]]
  void close( const name& owner, const symbol& symbol );

  [[eosio::action]]
  void setissuer(const name& issuer);

  [[eosio::action]]
  void setadmin(const name& admin);

  [[eosio::action]]
  void setwhite(const name& account, const bool& enabled);

  [[eosio::action]]
  void setbadgestore(name badgestore_contract);

  // 新增/修改规则（若 id == 0 则新增，否则修改对应 id）
  [[eosio::action]]
  void addrule(uint64_t id, const asset& threshold, const nsymbol& symbol, bool enabled);

  [[eosio::action]]
  void delrule(uint64_t id);

  [[eosio::action]] void notifyaward(const name& user,
                                  const vector<nasset>& packs,
                                  const string& memo);

  using notifyaward_action     = eosio::action_wrapper<"notifyaward"_n,     &cisum_token::notifyaward>;


  // -------- Helpers (static) --------
  static asset get_supply(const name& token_contract_account, const symbol_code& sym_code) {
    stats_t::idx_t statstable(token_contract_account, token_contract_account.value);
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
  void sub_balance(const name& owner, const asset& value, bool count_consumed = false);
  void add_balance(const name& owner, const asset& value, const name& ram_payer);

  inline void require_issuer(const name& actor) const {
    CHECKC(_gstate.issuer.value != 0, err::RECORD_NO_FOUND, "global issuer not set");
    CHECKC(actor == _gstate.issuer,   err::DID_NOT_AUTH,     "issuer only");
  }
  inline bool has_admin_auth() const {
    return has_auth(get_self()) ||
          (_gstate.admin.value != 0 && has_auth(_gstate.admin));
  }

  inline bool is_consumewl(const name& acc) const {
      redeem_whitelist_t::idx_t wtbl(get_self(), get_self().value);
      auto it = wtbl.find(acc.value);
      return (it != wtbl.end() && it->enabled);
  }





  inline bool in_whitelist(name acct) const{
    transfer_whitelist_t::idx_t wl(get_self(), get_self().value);
    auto it = wl.find(acct.value);
    return it != wl.end() && it->enabled;
  }


  void try_award_badges(const name& account, int64_t consumed_before, int64_t consumed_after);

};

} // namespace flon