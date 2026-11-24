#include "cisum.vault.hpp"

#include <flon/consts.hpp>
#include <flon/flon.token.hpp>
#include <flon.auth/flon.auth.hpp>
#include "base/utils.hpp"

namespace flon {

using eosio::asset;
using eosio::check;
using eosio::is_account;
using eosio::name;
using std::string;

void cisum_vault::require_paytype_perm(const name& submitter) const {
  require_auth(submitter);
  flonauth::checkrole_action checkrole(CISUMAUTH_CONTRACT, {get_self(), "active"_n});
  checkrole.send(get_self(), submitter, "paytypeManage");
}

bool cisum_vault::paytype_exists(const name& pay_type) const {
  paytypes_idx types(get_self(), get_self().value);
  const auto bytype = types.get_index<"bytype"_n>();
  return bytype.find(pay_type.value) != bytype.end();
}

void cisum_vault::addpaytype(const name& submitter, const name& pay_type, const string& desc) {
  require_paytype_perm(submitter);
  CHECKC(pay_type.value != 0, err::INVALID_FORMAT, "payment type cannot be empty");
  CHECKC(!paytype_exists(pay_type), err::INVALID_FORMAT, "payment type already exists");
  paytypes_idx types(get_self(), get_self().value);
  types.emplace(get_self(), [&](auto& row) {
    row.id = types.available_primary_key();
    row.pay_type = pay_type;
    row.desc = desc;
    row.created_at = current_time_point();
  });
}

void cisum_vault::delpaytype(const name& submitter, const name& pay_type) {
  require_paytype_perm(submitter);
  paytypes_idx types(get_self(), get_self().value);
  auto bytype = types.get_index<"bytype"_n>();
  const auto itr = bytype.find(pay_type.value);
  CHECKC(itr != bytype.end(), err::RECORD_NO_FOUND, "payment type not found");
  bytype.erase(itr);
}

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
  // 4) artistvote[:...]
  // 5) minivlog[:...]
  // 6) mallpay[:...]
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

  if (op == "artistvote") {
    return;
  }

  if (op == "minivlog") {
    return;
  }

  if (op == "shopping") {
    return;
  }
  if (op == "mallpay") {
    return;
  }
  if(paytype_exists(name{op})){
    return;
  }

  CHECKC(false, err::INVALID_FORMAT, string("unknown memo op: ") + string(op));
}

} // namespace flon
