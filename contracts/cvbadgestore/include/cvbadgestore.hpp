#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <string>
#include <vector>

#include "cvbadgestore.db.hpp"


namespace flon {

using eosio::name;
using eosio::contract;
using eosio::datastream;
using eosio::action_wrapper;
using std::string;
using std::vector;

class [[eosio::contract("cvbadgestore")]] cvbadgestore : public contract {
public:
  using contract::contract;

  cvbadgestore(name self, name code, datastream<const char*> ds)
  : contract(self, code, ds),
    _global(self, self.value)
  { _gstate = _global.exists() ? _global.get() : global_t{}; }

  ~cvbadgestore() { _global.set(_gstate, get_self()); }

  [[eosio::action]]
  void setadmin(const name& admin);
  [[eosio::action]]
  void setbadge(const name& badge_contract, const name& badge_from);
  [[eosio::action]]
  void addwhitelist(const name& account);
  [[eosio::action]]
  void delwhitelist(const name& account);
  [[eosio::action]]
  void createbadge(const name& submitter,
                   const int64_t& max_supply,
                   const nsymbol& symbol,
                   const string& token_uri,
                   const int64_t& issue_amount,
                   const string& memo);

  [[eosio::on_notify("*::notifyaward")]]
  void on_notifyaward(const name& user,
                    const vector<nasset>& packs,
                    const string& memo);
  [[eosio::action]]
  void reward(const name& submitter, const name& user,const std::vector<nasset>& packs,const std::string& memo) ;
private:
  global_singleton _global;
  global_t         _gstate;

  inline bool has_admin_auth() const {
    return has_auth(get_self()) || (_gstate.admin.value != 0 && has_auth(_gstate.admin));
  }
  inline void require_ready() const {
    CHECKC(is_account(_gstate.badge_contract), err::INVALID_FORMAT, "badge_bank not set or not exist");
    CHECKC(is_account(_gstate.badge_from), err::INVALID_FORMAT, "badge_from not set or not exist");
  }
  inline bool has_whitelist(const name& submitter) const {
    whitelist_t::idx_t wtbl(get_self(), get_self().value);
    auto it = wtbl.find(submitter.value);
    if (it == wtbl.end()) return false;
    return true;
  }
  void require_perm(const name& submitter, const string& perm) const;

};

} // namespace flon
