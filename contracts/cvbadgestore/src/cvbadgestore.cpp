#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <string>
#include <vector>

#include "cvbadgestore.hpp"
#include "cvbadgestore.db.hpp"
#include "flon.ntoken.hpp"
#include <flon.auth/flon.auth.hpp>
#include <flon/consts.hpp>

using std::string;
using std::vector;

namespace flon {

void cvbadgestore::require_perm(const name& submitter, const std::string& perm) const {
  require_auth(submitter);
  flonauth::checkrole_action(
    CISUMAUTH_CONTRACT,
    { get_self(), "active"_n }
  ).send(get_self(), submitter, perm);
}

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

void cvbadgestore::createbadge(const name& submitter,
                               const int64_t& max_supply,
                               const nsymbol& symbol,
                               const string& token_uri,
                               const int64_t& issue_amount,
                               const string& memo) {
  require_perm(submitter, "show");

  const name badge_contract = _gstate.badge_contract.value == 0
                             ? CVBADGE_CONTRACT
                             : _gstate.badge_contract;
  CHECKC(is_account(badge_contract), err::ACCOUNT_INVALID, "badge_contract not exist");
  CHECKC(max_supply > 0, err::NOT_POSITIVE, "max_supply must be positive");
  CHECKC(issue_amount > 0, err::NOT_POSITIVE, "issue_amount must be positive");
  CHECKC(issue_amount <= max_supply, err::INVALID_FORMAT, "issue_amount exceeds max_supply");
  CHECKC(symbol.nid != 0, err::INVALID_FORMAT, "invalid nsymbol");
  CHECKC(token_uri.size() <= 512, err::INVALID_FORMAT, "token_uri too long");
  CHECKC(memo.size() <= 256, err::INVALID_FORMAT, "memo too long");

  ntoken::create_action{
    badge_contract,
    { permission_level{ get_self(), "active"_n } }
  }.send(get_self(), max_supply, symbol, token_uri, get_self());

  ntoken::issue_action{
    badge_contract,
    { permission_level{ get_self(), "active"_n } }
  }.send(get_self(), nasset{ issue_amount, symbol }, memo);
}

void cvbadgestore::settokenuri(const name& submitter,
                               const uint64_t& symbid,
                               const string& token_uri) {
  require_perm(submitter, "show");

  const name badge_contract = _gstate.badge_contract.value == 0
                             ? CVBADGE_CONTRACT
                             : _gstate.badge_contract;
  CHECKC(is_account(badge_contract), err::ACCOUNT_INVALID, "badge_contract not exist");
  CHECKC(symbid != 0, err::INVALID_FORMAT, "invalid symbid");
  CHECKC(token_uri.size() <= 512, err::INVALID_FORMAT, "token_uri too long");

  ntoken::settokenuri_action{
    badge_contract,
    { permission_level{ get_self(), "active"_n } }
  }.send(symbid, token_uri);
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
  CHECKC(memo.size() <= 256,       err::INVALID_FORMAT,  "memo too long");
  CHECKC(_gstate.badge_contract.value != 0, err::RECORD_NO_FOUND, "badge_contract not set");
  CHECKC(_gstate.badge_from.value != 0,     err::RECORD_NO_FOUND, "badge_from not set");

  CHECKC(packs.size() <= 100, err::EXCEED_LIMIT, "too many packs");

  for (const auto& na : packs) {
    CHECKC(na.amount > 0,         err::NOT_POSITIVE,    "nasset amount must be positive");
    CHECKC(na.symbol.nid != 0,    err::INVALID_FORMAT,  "invalid nsymbol");
  }

  // 发放勋章
  NTOKEN_TRANSFER(_gstate.badge_contract, get_self(), user, packs, memo);

}

void cvbadgestore::reward(const name& submitter, const name& user,const std::vector<nasset>& packs,const std::string& memo) {
    require_auth(submitter);
    CHECKC(has_whitelist(submitter), err::DID_NOT_AUTH, "notifier not whitelisted");

    CHECKC(is_account(user),         err::ACCOUNT_INVALID, "user not exist");
    CHECKC(!packs.empty(),           err::INVALID_FORMAT,  "packs is empty");
    CHECKC(memo.size() <= 256,       err::INVALID_FORMAT,  "memo too long");

    CHECKC(_gstate.badge_contract.value != 0,
           err::RECORD_NO_FOUND, "badge_contract not set");
    CHECKC(_gstate.badge_from.value != 0,
           err::RECORD_NO_FOUND, "badge_from not set");

    CHECKC(packs.size() <= 100, err::EXCEED_LIMIT, "too many packs");

    for (const auto& na : packs) {
        CHECKC(na.amount > 0,
               err::NOT_POSITIVE, "nasset amount must be positive");
        CHECKC(na.symbol.nid != 0,
               err::INVALID_FORMAT, "invalid nsymbol");
    }

    // 3️⃣ 发放勋章（与 on_notifyaward 完全一致）
    NTOKEN_TRANSFER( _gstate.badge_contract,get_self(),  user,packs,memo );
}


} // namespace flon
