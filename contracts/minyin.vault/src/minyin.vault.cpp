#include "minyin.vault.hpp"

#include <flon/consts.hpp>
#include <flon/flon.token.hpp>
#include "base/utils.hpp"

namespace flon {

using eosio::asset;
using eosio::check;
using eosio::is_account;
using eosio::name;
using std::string;

void minyin_vault::require_admin() const {
  CHECKC(_gstate.admin.value != 0, err::RECORD_NO_FOUND, "admin not set");
  CHECKC(has_auth(get_self()) || has_auth(_gstate.admin), err::DID_NOT_AUTH, "admin/contract only");
}

void minyin_vault::init(const name& admin) {
  require_auth(get_self());

  CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin not exist");
  _gstate.admin = admin;
  _gstate.paused = false;
}

void minyin_vault::setadmin(const name& admin) {
  require_auth(get_self());
  CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin not exist");
  _gstate.admin = admin;
}

void minyin_vault::setpause(const bool& paused) {
  require_admin();
  _gstate.paused = paused;
}

void minyin_vault::ontransfer(const name& from, const name& to, const asset& quantity, const string& memo) {
  if (from == get_self() || to != get_self()) return;

  CHECKC(!_gstate.paused, err::UNDER_MAINTENANCE, "contract paused");
  CHECKC(get_first_receiver() == CISUM_CONTRACT, err::INVALID_FORMAT, "invalid token contract");
  CHECKC(quantity.is_valid(), err::INVALID_FORMAT, "invalid quantity");
  CHECKC(quantity.amount > 0, err::NOT_POSITIVE, "must transfer positive");
  CHECKC(quantity.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "symbol mismatch");
  CHECKC(memo.size() <= 256, err::INVALID_FORMAT, "memo too long");

  // memo formats:
  // 1) minyinvote[....]
  auto parts = ::split(string_view(memo), ":");
  CHECKC(!parts.empty(), err::INVALID_FORMAT, "empty memo");

  const auto op = parts[0];

  if (op == "minyinvote") {
    return;
  }


  CHECKC(false, err::INVALID_FORMAT, string("unknown memo op: ") + string(op));
}

} // namespace flon
