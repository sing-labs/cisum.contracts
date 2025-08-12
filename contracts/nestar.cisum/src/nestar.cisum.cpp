#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
#include <utils.hpp>
#include "nestar.cisum.hpp"
#include <string>

namespace flon {

using namespace eosio;
using std::string;

static inline int64_t pow10i(uint8_t p) {
  int64_t v = 1;
  while (p--) v *= 10;
  return v;
}

void nestar::setissuer(const name& issuer) {
    require_auth(get_self());
    CHECKC(is_account(issuer), err::ACCOUNT_INVALID, "issuer account not exist");
    _gstate.issuer = issuer;
}

void nestar::setrate(const uint64_t& rate) {
    require_auth(get_self());
    CHECKC(rate > 0, err::NOT_POSITIVE, "rate must be positive");

    _gstate.cisum_to_nestar_rate = rate;
}

void nestar::setblacklist(const name& account, const bool& banned) {
    require_auth(get_self());
    CHECKC(is_account(account), err::ACCOUNT_INVALID, "account not exist");

    if (banned) {
        _gstate.blacklist.insert(account);
    } else {
        _gstate.blacklist.erase(account);
    }
}

void nestar::create(const name& issuer, const asset& maximum_supply)
{
    require_auth(get_self()); // 仍由合约账号发布/初始化

    // 新增：issuer 必须等于全局发行方（避免填错）
    require_issuer(issuer);

    CHECKC(maximum_supply.symbol == NESTAR_SYMBOL,  err::SYMBOL_MISMATCH, "symbol must be NESTAR");
    CHECKC(maximum_supply.symbol.is_valid(),        err::INVALID_FORMAT,  "invalid symbol name");
    CHECKC(maximum_supply.is_valid(),               err::INVALID_FORMAT,  "invalid supply");
    CHECKC(maximum_supply.amount > 0,               err::NOT_POSITIVE,    "max-supply must be positive");
    CHECKC(is_account(issuer),                      err::ACCOUNT_INVALID, "issuer account not exist");

    stats_t::idx_t statstable(get_self() ,get_self().value);
    auto existing = statstable.find(NESTAR_SYMBOL.code().raw());
    CHECKC(existing == statstable.end(),            err::REDPACK_EXIST,   "NESTAR already exists");

    statstable.emplace(get_self(), [&](auto& s){
      s.supply.symbol = maximum_supply.symbol;
      s.max_supply    = maximum_supply;
      s.created_at    = current_time_point();
    });
}


void nestar::issue(const name& to, const asset& quantity, const string& memo)
{
    require_auth(to);
    require_issuer(to);

    CHECKC(memo.size() <= 256,                    err::INVALID_FORMAT,     "memo has more than 256 bytes");
    CHECKC(quantity.is_valid(),                   err::INVALID_FORMAT,     "invalid quantity");
    CHECKC(quantity.amount > 0,                   err::NOT_POSITIVE,       "must issue positive quantity");
    CHECKC(quantity.symbol == NESTAR_SYMBOL,      err::SYMBOL_MISMATCH,    "symbol must be NESTAR");


    stats_t::idx_t statstable(get_self(),get_self().value);
    auto existing = statstable.find(NESTAR_SYMBOL.code().raw());
    CHECKC(existing != statstable.end(),          err::RECORD_NO_FOUND,    "token with symbol does not exist, create token first");
    const auto& st = *existing;

    CHECKC(quantity.amount <= (st.max_supply.amount - st.supply.amount),
                                                  err::AMOUNT_TOO_LARGE,   "quantity exceeds available supply");

    statstable.modify(st, same_payer, [&](auto& s){
        s.supply += quantity;
    });

    add_balance(to, quantity, to);
}

void nestar::retire(const asset& quantity, const string& memo)
{
    auto sym = quantity.symbol;
    CHECKC(sym.is_valid(),                        err::INVALID_FORMAT,     "invalid symbol name");
    CHECKC(memo.size() <= 256,                    err::INVALID_FORMAT,     "memo has more than 256 bytes");

    stats_t::idx_t statstable(get_self(),get_self().value);
    auto existing = statstable.find(sym.code().raw());
    CHECKC(existing != statstable.end(),          err::RECORD_NO_FOUND,    "token with symbol does not exist");
    const auto& st = *existing;

    require_auth(_gstate.issuer);
    CHECKC(quantity.is_valid(),                   err::INVALID_FORMAT,     "invalid quantity");
    CHECKC(quantity.amount > 0,                   err::NOT_POSITIVE,       "must retire positive quantity");
    CHECKC(quantity.symbol == st.supply.symbol,   err::SYMBOL_MISMATCH, "symbol precision mismatch");

    statstable.modify(st, same_payer, [&](auto& s) {
        s.supply -= quantity;
    });

    sub_balance(_gstate.issuer, quantity);
}


// ======== transfer ========
// 只允许：
// 1) 发行者 -> 任意账户（发放/奖励）
// 2) 用户   -> 艺人白名单账户（打赏/周边购买）
// 禁止其它路径（例如用户->用户、艺人->用户、艺人->艺人等）
// 签名者：from
void nestar::transfer(const name& from,
                      const name& to,
                      const asset& quantity,
                      const string& memo)
{
    CHECKC(from != to,                      err::INVALID_FORMAT,      "cannot transfer to self");
    require_auth(from);
    CHECKC(is_account(to),                  err::ACCOUNT_INVALID,     ("to account does not exist: " + to.to_string()));
    CHECKC(memo.size() <= 256,              err::INVALID_FORMAT,      "memo has more than 256 bytes");

    CHECKC(quantity.symbol == NESTAR_SYMBOL,err::SYMBOL_MISMATCH,     "symbol must be NESTAR");
    CHECKC(quantity.is_valid(),             err::INVALID_FORMAT,      "invalid quantity");
    CHECKC(quantity.amount > 0,             err::NOT_POSITIVE,        "must transfer positive quantity");

    bool allowed = false;
    if (from == _gstate.issuer) {
        require_issuer(from);
        allowed = true;                            // 发行者 -> 任意账户
    } else if (is_artist_enabled(to)) {
        allowed = true;                            // 用户 -> 艺人（白名单）
    }
    CHECKC(allowed,                           err::DID_NOT_AUTH,       "transfer not allowed: only issuer->anyone or user->artist");
    auto payer = has_auth(to) ? to : from;
    sub_balance(from, quantity);
    add_balance(to, quantity, payer);

    require_recipient(from);
    require_recipient(to);
}

void nestar::sub_balance(const name& owner, const asset& value)
{
    CHECKC(value.symbol == NESTAR_SYMBOL, err::SYMBOL_MISMATCH, "symbol must be NESTAR");
    CHECKC(value.is_valid(),              err::INVALID_FORMAT,   "invalid quantity");
    CHECKC(value.amount > 0,              err::NOT_POSITIVE,     "amount must be positive");

    account_t::idx_t from_acnts(get_self(), owner.value);
    const auto& from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
    CHECKC(from.balance.amount >= value.amount, err::INSUFFICIENT_QUANTITY, "overdrawn balance");

    from_acnts.modify(from, same_payer, [&](auto& a) {
        a.balance -= value;
    });
}

void nestar::add_balance(const name& owner, const asset& value, const name& ram_payer)
{
    CHECKC(value.symbol == NESTAR_SYMBOL, err::SYMBOL_MISMATCH, "symbol must be NESTAR");
    CHECKC(value.is_valid(),              err::INVALID_FORMAT,   "invalid quantity");
    CHECKC(value.amount > 0,              err::NOT_POSITIVE,     "amount must be positive");

    account_t::idx_t to_acnts(get_self(), owner.value);
    auto it = to_acnts.find(value.symbol.code().raw());
    if (it == to_acnts.end()) {
        to_acnts.emplace(ram_payer, [&](auto& a) {
            a.balance    = value;
            // a.allow_send = false;
            // a.allow_recv = false;
        });
    } else {
        to_acnts.modify(it, same_payer, [&](auto& a) {
            a.balance += value;
        });
    }
}

void nestar::open(const name& owner, const symbol& sym, const name& ram_payer)
{
    require_auth(ram_payer);
    CHECKC(is_account(owner),                err::ACCOUNT_INVALID,     "owner account does not exist");
    CHECKC(sym == NESTAR_SYMBOL,             err::SYMBOL_MISMATCH,     "symbol must be NESTAR");

    stats_t::idx_t statstable(get_self(),get_self().value);
    const auto& st = statstable.get(sym.code().raw(), "symbol does not exist");
    CHECKC(st.supply.symbol == sym,          err::SYMBOL_MISMATCH,     "symbol precision mismatch");

    account_t::idx_t acnts(get_self(), owner.value);
    auto it = acnts.find(sym.code().raw());
    if (it == acnts.end()) {
        acnts.emplace(ram_payer, [&](auto& a) {
            a.balance    = asset{0, sym};
            a.allow_send = false;
            a.allow_recv = false;
        });
    }
}

// 管理艺人白名单（enable/disable）
void nestar::setartist(const name& artist, bool enabled)
{
    require_auth(get_self());
    CHECKC(is_account(artist),               err::ACCOUNT_INVALID,     "artist account does not exist");

    artist_t::idx_t atbl(get_self(), get_self().value);
    auto it = atbl.find(artist.value);
    if (it == atbl.end()) {
        atbl.emplace(get_self(), [&](auto& r) {
            r.account = artist;
            r.enabled = enabled;
        });
    } else {
        atbl.modify(it, same_payer, [&](auto& r) {
            r.enabled = enabled;
        });
    }
}

void nestar::setacctperms(const name& issuer, const name& to, const symbol& symbol, const bool& allowsend, const bool& allowrecv)
{
    require_auth(issuer);
    require_issuer(issuer);

    CHECKC(is_account(to),                   err::ACCOUNT_INVALID,     ("to account does not exist: " + to.to_string()));
    CHECKC(symbol == NESTAR_SYMBOL,          err::SYMBOL_MISMATCH,     "invalid NESTAR symbol");

    account_t::idx_t acnts(get_self(), to.value);
    auto it = acnts.find(symbol.code().raw());
    if (it == acnts.end()) {
        acnts.emplace(issuer, [&](auto& a) {
            a.balance    = asset(0, NESTAR_SYMBOL);
            a.allow_send = allowsend;
            a.allow_recv = allowrecv;
        });
    } else {
        acnts.modify(it, issuer, [&](auto& a) {
            a.allow_send = allowsend;
            a.allow_recv = allowrecv;
        });
    }
}

// 监听用户把 CISUM 转到本合约：按固定比例兑换为 NESTAR 并发放给用户
void nestar::on_transfer(const name& from, const name& to, const asset& quantity, const string& memo) {
  if (to != get_self() || from == get_self()) return;

  CHECKC(get_first_receiver() == CISUM_CONTRACT,            err::ACCOUNT_INVALID, "only accept transfer from CISUM contract");
  CHECKC(_gstate.blacklist.count(from) == 0,                err::ACCOUNT_INVALID,       "from is blacklisted");
  CHECKC(quantity.symbol == CISUM_SYMBOL,                   err::SYMBOL_MISMATCH,       "incoming must be CISUM");
  CHECKC(quantity.is_valid(),                               err::INVALID_FORMAT,        "invalid CISUM quantity");
  CHECKC(quantity.amount > 0,                               err::NOT_POSITIVE,          "CISUM amount must be positive");

  // 确保已设置兑换比例
  CHECKC(_gstate.cisum_to_nestar_rate > 0,                  err::INVALID_FORMAT,        "exchange rate not set");

  stats_t::idx_t statstable(get_self(),get_self().value);
  auto sitr = statstable.find(NESTAR_SYMBOL.code().raw());
  CHECKC(sitr != statstable.end(),                          err::RECORD_NO_FOUND,       "NESTAR not created yet");
  const auto& st = *sitr;

  const uint64_t rate = _gstate.cisum_to_nestar_rate;
  const int64_t  cisum_prec = pow10i(CISUM_SYMBOL.precision());
  int64_t out_amount = (quantity.amount * (int64_t)rate) / cisum_prec;
  CHECKC(out_amount > 0,                                    err::AMOUNT_TOO_SMALL,      "CISUM amount too small to convert");

  asset out_nestar(out_amount, NESTAR_SYMBOL);
  CHECKC(out_nestar.amount <= (st.max_supply.amount - st.supply.amount),
                                                            err::AMOUNT_TOO_LARGE,      "not enough NESTAR remaining in max_supply");

  statstable.modify(st, same_payer, [&](auto& s){ s.supply += out_nestar; });
  add_balance(from, out_nestar, get_self());

   exchange_t::idx_t xchg(get_self(), get_self().value);
  xchg.emplace(get_self(), [&](auto& r){
    r.id          = xchg.available_primary_key();
    r.user        = from;
    r.in_cisum    = quantity;
    r.out_nestar  = out_nestar;
    r.memo        = memo;
    r.created_at  = current_time_point();
  });

  require_recipient(from);
}





} /// namespace eosio
