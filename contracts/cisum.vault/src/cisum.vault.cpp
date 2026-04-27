#include "cisum.vault.hpp"

#include <flon/consts.hpp>
#include <flon/flon.token.hpp>
#include "base/utils.hpp"

namespace flon {

using eosio::asset;
using eosio::check;
using eosio::is_account;
using eosio::name;
using std::string;

void cisum_vault::require_admin() const {
  CHECKC(_gstate.admin.value != 0, err::RECORD_NO_FOUND, "admin not set");
  CHECKC(has_auth(get_self()) || has_auth(_gstate.admin), err::DID_NOT_AUTH, "admin/contract only");
}

void cisum_vault::init(const name& admin) {
  require_auth(get_self());

  CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin not exist");
  _gstate.admin = admin;
  _gstate.paused = false;
}

void cisum_vault::setadmin(const name& admin) {
  require_auth(get_self());
  CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin not exist");
  _gstate.admin = admin;
}

void cisum_vault::setpause(const bool& paused) {
  require_admin();
  _gstate.paused = paused;
}

void cisum_vault::ontransfer(const name& from, const name& to, const asset& quantity, const string& memo) {
  if (from == get_self() || to != get_self()) return;

  CHECKC(!_gstate.paused, err::UNDER_MAINTENANCE, "contract paused");
  CHECKC(get_first_receiver() == CISUM_CONTRACT, err::INVALID_FORMAT, "invalid token contract");
  CHECKC(quantity.is_valid(), err::INVALID_FORMAT, "invalid quantity");
  CHECKC(quantity.amount > 0, err::NOT_POSITIVE, "must transfer positive");
  CHECKC(quantity.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "symbol mismatch");
  CHECKC(memo.size() <= 256, err::INVALID_FORMAT, "memo too long");

  // memo formats:
  // 1) points[:uid][:ref]
  // 2) livepay:room_id[:...]
  // 3) cisumptxchg:points
  auto parts = ::split(string_view(memo), ":");
  CHECKC(!parts.empty(), err::INVALID_FORMAT, "empty memo");

  const auto op = parts[0];

  if (op == "points") {
    return;
  }

  if (op == "livepay") {
    CHECKC(parts.size() >= 2, err::INVALID_FORMAT, "livepay memo requires room_id, e.g. livepay:123");
    const auto room_id = parts[1];
    CHECKC(!room_id.empty(), err::INVALID_FORMAT, "room_id must not be empty");
    return;
  }

  if (op == "cisumptxchg") {
    CHECKC(parts.size() >= 2, err::INVALID_FORMAT, "cisumptxchg memo must be cisumptxchg:<points>:.....");
    const auto points = parts[1];
    CHECKC(is_numeric(points), err::INVALID_FORMAT, "cisumptxchg points must be numeric");
    CHECKC(to_uint64(points) > 0, err::NOT_POSITIVE, "cisumptxchg points must be positive");
    return;
  }

  CHECKC(false, err::INVALID_FORMAT, string("unknown memo op: ") + string(op));
}

} // namespace flon
