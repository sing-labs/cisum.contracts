#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <string>
#include <vector>

#include "cvbadgestore.hpp"
#include "cvbadgestore.db.hpp"
#include "flon.ntoken.hpp" 

using std::string;
using std::vector;

namespace flon {

void cvbadgestore::setadmin(const name& admin) {
  require_auth(get_self());
  CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin not exist");
  _gstate.admin = admin;
}

void cvbadgestore::setbadge(const name& badge_contract, const name& badge_from) {
  require_auth(get_self());
  CHECKC(is_account(badge_contract),        err::ACCOUNT_INVALID, "badge_bank (ntoken contract) not exist");
  CHECKC(is_account(badge_from),            err::ACCOUNT_INVALID, "badge account not exist");
  _gstate.badge_contract  = badge_contract;
  _gstate.badge_from      = badge_from;
}

void cvbadgestore::addwhitelist(const name& contract_account) {
  CHECKC(has_admin_auth(),                  err::DID_NOT_AUTH,    "admin/contract only");
  CHECKC(is_account(contract_account),      err::ACCOUNT_INVALID, "account not exist");

  whitelist_t::idx_t wtbl(get_self(), get_self().value);
  auto it = wtbl.find(contract_account.value);
  CHECKC(it == wtbl.end(), err::REDPACK_EXIST, "already in whitelist");

  wtbl.emplace(get_self(), [&](auto& r){
    r.contract = contract_account;
    r.added_at = current_time_point();
  });
}

void cvbadgestore::delwhitelist(const name& account) {
  CHECKC(has_admin_auth(), err::DID_NOT_AUTH, "admin/contract only");
  whitelist_t::idx_t wtbl(get_self(), get_self().value);
  auto it = wtbl.find(account.value);
  CHECKC(it != wtbl.end(), err::RECORD_NO_FOUND, "not in whitelist");
  wtbl.erase(it);
}

void cvbadgestore::on_notifyaward(const name& user,
                                const std::vector<nasset>& packs,
                                const std::string& memo)
{

  const name submitter = get_first_receiver();

  CHECKC(has_whitelist(submitter), err::DID_NOT_AUTH, "notifier not whitelisted");

  require_ready();
  CHECKC(is_account(user),         err::ACCOUNT_INVALID, "user not exist");
  CHECKC(!packs.empty(),           err::INVALID_FORMAT,  "packs is empty");

  for (const auto& na : packs) {
    CHECKC(na.amount > 0,         err::NOT_POSITIVE,    "nasset amount must be positive");
    CHECKC(na.symbol.raw() != 0,  err::INVALID_FORMAT,  "invalid nsymbol");
  }


  flon::ntoken::transfer_action{
    _gstate.badge_contract,
    { permission_level{ _gstate.badge_from, "active"_n } }
  }.send(_gstate.badge_from, user, packs, memo);

}


} // namespace flon
