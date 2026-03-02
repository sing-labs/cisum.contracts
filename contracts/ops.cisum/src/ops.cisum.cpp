#include "ops.cisum.hpp"

#include <flon/consts.hpp>
#include <flon/flon.token.hpp>
#include "base/utils.hpp"

#include <cerrno>
#include <cstdlib>
#include <limits>

namespace flon {

using eosio::asset;
using eosio::check;
using eosio::current_time_point;
using eosio::is_account;
using eosio::name;
using std::string;
using std::vector;

uint64_t ops_cisum::to_u64(string_view s, string_view err_title) {
  errno = 0;
  std::string tmp{s};
  char* end = nullptr;
  unsigned long long v = std::strtoull(tmp.c_str(), &end, 10);
  CHECKC(errno == 0 && end != nullptr && *end == '\0', err::INVALID_FORMAT,
         string(err_title) + ": invalid uint64");
  CHECKC(v <= std::numeric_limits<uint64_t>::max(), err::INVALID_FORMAT,
         string(err_title) + ": overflow");
  return static_cast<uint64_t>(v);
}

void ops_cisum::require_admin() const {
  CHECKC(_gstate.admin.value != 0, err::RECORD_NO_FOUND, "admin not set");
  CHECKC(has_auth(get_self()) || has_auth(_gstate.admin), err::DID_NOT_AUTH, "admin/contract only");
}

void ops_cisum::init(const name& admin) {
  require_auth(get_self());

  CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin not exist");
  _gstate.admin = admin;
  _gstate.paused = false;
}

void ops_cisum::setadmin(const name& admin) {
  require_auth(get_self());
  CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin not exist");
  _gstate.admin = admin;
}

void ops_cisum::setpause(const bool& paused) {
  require_admin();
  _gstate.paused = paused;
}

void ops_cisum::ontransfer(const name& from, const name& to, const asset& quantity, const string& memo) {
  if (from == get_self() || to != get_self()) return;

  CHECKC(!_gstate.paused, err::UNDER_MAINTENANCE, "contract paused");
  CHECKC(get_first_receiver() == CISUM_CONTRACT, err::INVALID_FORMAT, "invalid token contract");
  CHECKC(quantity.is_valid(), err::INVALID_FORMAT, "invalid quantity");
  CHECKC(quantity.amount > 0, err::NOT_POSITIVE, "must transfer positive");
  CHECKC(quantity.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "symbol mismatch");
  CHECKC(memo.size() <= 256, err::INVALID_FORMAT, "memo too long");

  // memo formats:
  // 1) points[:uid][:ref]     -> record deposit for off-chain points exchange
  // 2) livepay:room_id[:...] -> record live room payment (funds stay in this contract)
  auto parts = ::split(string_view(memo), ":");
  CHECKC(!parts.empty(), err::INVALID_FORMAT, "empty memo");

  const auto now = current_time_point();
  const auto op = parts[0];

  if (op == "points") {
    // allow: "points" or "points:..."
    payment_t::idx_t pays(get_self(), get_self().value);
    CHECKC(_gstate.last_pay_id < std::numeric_limits<uint64_t>::max(), err::EXCEED_LIMIT, "pay id overflow");
    _gstate.last_pay_id += 1;
    const uint64_t id = _gstate.last_pay_id;
    pays.emplace(get_self(), [&](auto& r) {
      r.id = id;
      r.payer = from;
      r.kind = "exchange"_n;
      r.quantity = quantity;
      r.memo = memo;
      r.created_at = now;
    });
    return;
  }

  if (op == "livepay") {
    CHECKC(parts.size() >= 2, err::INVALID_FORMAT, "livepay memo requires room_id, e.g. livepay:123");
    const uint64_t room_id = to_u64(parts[1], "room_id");
    CHECKC(room_id > 0, err::INVALID_FORMAT, "room_id must be > 0");

    payment_t::idx_t pays(get_self(), get_self().value);
    CHECKC(_gstate.last_pay_id < std::numeric_limits<uint64_t>::max(), err::EXCEED_LIMIT, "pay id overflow");
    _gstate.last_pay_id += 1;
    const uint64_t id = _gstate.last_pay_id;
    pays.emplace(get_self(), [&](auto& r) {
      r.id = id;
      r.payer = from;
      r.kind = "livepay"_n;
      r.quantity = quantity;
      r.memo = memo;
      r.created_at = now;
    });
    return;
  }

  CHECKC(false, err::INVALID_FORMAT, string("unknown memo op: ") + string(op));
}

} // namespace flon
