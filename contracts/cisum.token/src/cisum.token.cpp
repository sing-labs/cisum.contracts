#include "cisum.token.hpp"

#include "cvbadge.nft.db.hpp"
#include "flon/consts.hpp"

namespace flon {

using namespace eosio;
using std::string;

void cisum_token::init(name issuer, name admin, name badgestore_contract) {
    require_auth(get_self());

    _gstate.issuer             = issuer;
    _gstate.admin              = admin;
    _gstate.badgestore_contract = badgestore_contract;
}

void cisum_token::addwhitelist(const name& account) {
    CHECKC( has_auth( _self ) || has_auth( _gstate.admin ), err::NO_AUTH, "admin/contract only" );
    CHECKC(is_account(account), err::ACCOUNT_INVALID, "account not exist");

    transfer_whitelist_t::idx_t tbl(get_self(), get_self().value);
    auto it = tbl.find(account.value);
    if (it == tbl.end()) {
        tbl.emplace(get_self(), [&](auto& row){
            row.account = account;
            row.enabled = true;
        });
    } else {
        tbl.modify(it, same_payer, [&](auto& row){ row.enabled = true; });
    }
}

void cisum_token::delwhitelist(const name& account) {
    CHECKC( has_auth( _self ) || has_auth( _gstate.admin ), err::NO_AUTH, "admin/contract only" );
    CHECKC(is_account(account), err::ACCOUNT_INVALID, "account not exist");

    transfer_whitelist_t::idx_t tbl(get_self(), get_self().value);
    auto it = tbl.find(account.value);
    check(it != tbl.end(), "whitelist not found");
    tbl.erase(it);
}


void cisum_token::addconsumewl(const name& account) {
    CHECKC( has_auth( _self ) || has_auth( _gstate.admin ), err::NO_AUTH, "admin/contract only" );
    CHECKC(is_account(account), err::ACCOUNT_INVALID, "account not exist");

    redeem_whitelist_t::idx_t wl(get_self(), get_self().value);
    auto it = wl.find(account.value);
    if (it == wl.end()) {
        wl.emplace(get_self(), [&](auto& r){ r.account = account; r.enabled = true; });
    } else {
        wl.modify(it, same_payer, [&](auto& r){ r.enabled = true; });
    }
}


void cisum_token::delconsumewl(const name& account) {
    CHECKC( has_auth( _self ) || has_auth( _gstate.admin ), err::NO_AUTH, "admin/contract only" );
    CHECKC(is_account(account), err::ACCOUNT_INVALID, "account not exist");

    redeem_whitelist_t::idx_t wl(get_self(), get_self().value);
    auto it = wl.find(account.value);
    check(it != wl.end(), "cwhitelist: account not found");
    wl.erase(it);
}




void cisum_token::create(const name& issuer, const asset& maximum_supply)
{
    require_auth(get_self());
    require_issuer(issuer);

    CHECKC(maximum_supply.symbol.is_valid(),        err::INVALID_FORMAT,  "invalid symbol");
    CHECKC(maximum_supply.is_valid(),               err::INVALID_FORMAT,  "invalid supply");
    CHECKC(maximum_supply.amount > 0,               err::NOT_POSITIVE,    "max-supply must be positive");
    CHECKC(is_account(issuer),                      err::ACCOUNT_INVALID, "issuer account not exist");
    auto sym = maximum_supply.symbol;
    stats_t::idx_t statstable(get_self(), sym.code().raw() );
    auto itr = statstable.find(sym.code().raw());
    CHECKC(itr == statstable.end(),                 err::REDPACK_EXIST,   "token already exists");

    statstable.emplace(get_self(), [&](auto& s){
      s.supply.symbol = maximum_supply.symbol;
      s.max_supply    = maximum_supply;
      s.created_at    = current_time_point();
      s.paused        = false;
    });
}


void cisum_token::issue(const name& to, const asset& quantity, const std::string& memo)
{
    auto sym = quantity.symbol;
    CHECKC( has_auth(to) || has_auth(POH_CONTRACT)|| has_auth(POS_CONTRACT),
                                                err::DID_NOT_AUTH, "missing auth (issuer or poh required or pos required)" );
    CHECKC( to == _gstate.issuer || to == POH_CONTRACT || to == POS_CONTRACT,
                                                err::ACCOUNT_INVALID, "to must be issuer/POH/POS" );

    CHECKC(memo.size() <= 256,                   err::INVALID_FORMAT,  "memo too long");

    CHECKC(quantity.is_valid(),                  err::INVALID_FORMAT,  "invalid quantity");
    CHECKC(quantity.amount > 0,                  err::NOT_POSITIVE,    "must issue positive quantity");

    stats_t::idx_t statstable(get_self(), sym.code().raw());
    auto itr = statstable.find(sym.code().raw());
    CHECKC(itr != statstable.end(),              err::RECORD_NO_FOUND, "token not created");
    const auto& st = *itr;

    CHECKC(quantity.amount <= (st.max_supply.amount - st.supply.amount),
                                              err::AMOUNT_TOO_LARGE,  "quantity exceeds available supply");

    statstable.modify(st, same_payer, [&](auto& s){
      s.supply += quantity;
    });

    add_balance(to, quantity, to);
}

void cisum_token::retire(const asset& quantity, const string& memo)
{
    CHECKC(quantity.symbol.is_valid(),           err::INVALID_FORMAT,  "invalid symbol");
    CHECKC(memo.size() <= 256,                   err::INVALID_FORMAT,  "memo too long");
    CHECKC(quantity.is_valid(),                  err::INVALID_FORMAT,  "invalid quantity");
    CHECKC(quantity.amount > 0,                  err::NOT_POSITIVE,    "must retire positive quantity");

    auto sym = quantity.symbol;
    stats_t::idx_t statstable(get_self(), sym.code().raw());
    auto itr = statstable.find(sym.code().raw());
    CHECKC(itr != statstable.end(),              err::RECORD_NO_FOUND, "token not created");
    const auto& st = *itr;
    CHECKC(itr->supply.symbol == quantity.symbol, err::SYMBOL_MISMATCH, "symbol mismatch");

    require_auth(_gstate.issuer);

    statstable.modify(st, same_payer, [&](auto& s) {
      s.supply -= quantity;
    });

    sub_balance(_gstate.issuer, quantity, /*count_consumed=*/false);
}


// 允许：
// 1) 发行者 -> 任意账户或合约
// 2) 普通用户 -> 白名单账户/合约（功能白名单）
// 在 2) 的情况下，from 的本次支出会计入 consumed 并检查勋章发放
void cisum_token::transfer(const name& from,
                      const name& to,
                      const asset& quantity,
                      const std::string& memo)
{
    CHECKC(from != to,                         err::INVALID_FORMAT,   "cannot transfer to self");
    require_auth(from);
    CHECKC(is_account(to),                     err::ACCOUNT_INVALID,  "to account not exist");


    auto sym = quantity.symbol.code();
    stats_t::idx_t statstable(get_self(), sym.raw());
    auto itr = statstable.find(sym.raw());

    CHECKC(quantity.is_valid(),                err::INVALID_FORMAT,   "invalid quantity");
    CHECKC(quantity.amount > 0,                err::NOT_POSITIVE,     "must transfer positive quantity");
    CHECKC(quantity.symbol == itr->supply.symbol, err::INVALID_FORMAT, "symbol precision mismatch");
    CHECKC(memo.size() <= 256,                 err::INVALID_FORMAT,   "memo too long");

    bool allowed = false;
    bool count_consumed = false;


    // issuer 或 whitelist 用户转账 → 允许，但不计消费
    if (from == _gstate.issuer || in_whitelist(from)) {
        allowed = true;
    }

    // 转给消费白名单合约 → 允许，并计消费
    if (is_consumewl(to)) {
        allowed = true;
        count_consumed = true;
    }

    CHECKC(allowed, err::DID_NOT_AUTH, "transfer not allowed: issuer->any or user->consumewl");

    int64_t consumed_before = 0;
    if (count_consumed) {
        account_t::idx_t facnts(get_self(), from.value);
        const auto& frow = facnts.get(quantity.symbol.code().raw(), "no balance row for sender");
        consumed_before = frow.consumed.amount;
    }

    auto payer = has_auth(to) ? to : from;
    sub_balance(from, quantity,count_consumed);
    add_balance(to, quantity, payer);

    if (count_consumed) {
        account_t::idx_t acnts(get_self(), from.value);
        const auto& row = acnts.get(quantity.symbol.code().raw(), "no balance row after transfer");
        const int64_t consumed_after = row.consumed.amount;

        try_award_badges(from, consumed_before, consumed_after);
    }

    require_recipient(from);
    require_recipient(to);
}


void cisum_token::setissuer(const name& issuer) {
    require_auth(get_self());
    CHECKC(is_account(issuer),      err::ACCOUNT_INVALID, "issuer account not exist");
    _gstate.issuer = issuer;
}

void cisum_token::setadmin(const name& admin)
{
    require_auth(get_self());
    CHECKC(is_account(admin),       err::ACCOUNT_INVALID, "admin not exist");
    _gstate.admin = admin;
}

void cisum_token::setwhite(const name& account, const bool& enabled)
{
    require_auth(get_self());
    CHECKC(is_account(account),     err::ACCOUNT_INVALID, "account not exist");

    transfer_whitelist_t::idx_t wtbl(get_self(), get_self().value);
    auto it = wtbl.find(account.value);
    if (it == wtbl.end()) {
        wtbl.emplace(get_self(), [&](auto& r){
            r.account = account;
            r.enabled = enabled;
        });
    } else {
        wtbl.modify(it, same_payer, [&](auto& r){
            r.enabled = enabled;
        });
    }
}

void cisum_token::addrule(uint64_t id, const asset& threshold, const nsymbol& symbol, bool enabled) {
    CHECKC(has_admin_auth(),                    err::DID_NOT_AUTH,     "admin/contract only");
    CHECKC(threshold.amount > 0,                err::NOT_POSITIVE,     "threshold must be positive");
    CHECKC(symbol.nid != 0,                     err::INVALID_FORMAT,   "badge symbol required");

    auto symcode = threshold.symbol.code();
    stats_t::idx_t statstable(get_self(), symcode.raw());
    auto itr = statstable.find(symcode.raw());
    CHECKC(itr != statstable.end(), err::RECORD_NO_FOUND, "threshold token not exist");

    flon::nstats_t::idx_t nstats_tbl(CVBADGE_CONTRACT, CVBADGE_CONTRACT.value);
    auto it_symbol = nstats_tbl.find(symbol.nid);

    CHECKC(it_symbol != nstats_tbl.end(), err::RECORD_NO_FOUND, "badge symbol not exist in badge nft");

    badge_rule_t::idx_t rtbl(get_self(), get_self().value);
    auto by_symbol = rtbl.get_index<"bysymbol"_n>();

    if (id == 0) {

        CHECKC(by_symbol.find(symbol.nid) == by_symbol.end(),
               err::REDPACK_EXIST, "rule for this badge symbol already exists");

        rtbl.emplace(get_self(), [&](auto& r){
            r.id         = rtbl.available_primary_key();
            r.threshold  = threshold;
            r.symbol     = symbol;
            r.enabled    = enabled;
            r.created_at = current_time_point();
        });
    } else {
        auto it = rtbl.find(id);
        CHECKC(it != rtbl.end(), err::RECORD_NO_FOUND, "badge rule not found");

        if (it->symbol.nid != symbol.nid) {
            CHECKC(by_symbol.find(symbol.nid) == by_symbol.end(),
                   err::REDPACK_EXIST, "rule for this badge symbol already exists");
        }

        rtbl.modify(it, same_payer, [&](auto& r){
            r.threshold = threshold;
            r.symbol    = symbol;
            r.enabled   = enabled;
        });
    }
}

void cisum_token::delrule(uint64_t id)
{
    CHECKC(has_admin_auth(),          err::DID_NOT_AUTH, "admin/contract only");
    badge_rule_t::idx_t rtbl(get_self(), get_self().value);
    auto it = rtbl.find(id);
    CHECKC(it != rtbl.end(),          err::RECORD_NO_FOUND, "badge rule not found");
    rtbl.erase(it);
}

void cisum_token::setbadgestore(name badgestore_contract) {
  require_auth(get_self());
  CHECKC(is_account(badgestore_contract), err::ACCOUNT_INVALID, "badge contract not exist");
  _gstate.badgestore_contract = badgestore_contract;
}

void cisum_token::sub_balance(const name& owner, const asset& value, bool count_consumed)
{
    CHECKC(value.is_valid(),              err::INVALID_FORMAT,   "invalid quantity");
    CHECKC(value.amount > 0,              err::NOT_POSITIVE,     "amount must be positive");

    account_t::idx_t from_acnts(get_self(), owner.value);
    const auto& from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
    CHECKC(from.balance.amount >= value.amount, err::INSUFFICIENT_QUANTITY, "overdrawn balance");

    from_acnts.modify(from, same_payer, [&](auto& a) {
        a.balance -= value;
        if (count_consumed) {
            a.consumed += value;
        }
        a.updated_at = current_time_point();
    });
}

void cisum_token::add_balance(const name& owner, const asset& value, const name& ram_payer)
{
    CHECKC(value.is_valid(),              err::INVALID_FORMAT,  "invalid quantity");
    CHECKC(value.amount > 0,              err::NOT_POSITIVE,    "amount must be positive");

    account_t::idx_t acnts(get_self(), owner.value);
    auto it = acnts.find(value.symbol.code().raw());
    if (it == acnts.end()) {
        acnts.emplace(ram_payer, [&](auto& a){
        a.balance    = value;
        a.consumed   = asset{0, value.symbol};
        a.created_at = current_time_point();
        a.updated_at = a.created_at;
        });
    } else {
        acnts.modify(it, same_payer, [&](auto& a){
        a.balance   += value;
        a.updated_at = current_time_point();
        });
    }
}

void cisum_token::open(const name& owner, const symbol& sym, const name& ram_payer)
{

    require_auth(ram_payer);
    CHECKC(is_account(owner),                err::ACCOUNT_INVALID, "owner not exist");

    auto sym_code_raw = sym.code().raw();
    stats_t::idx_t statstable(get_self(), sym_code_raw);
    const auto& st = statstable.get(sym_code_raw, "symbol not created");
    CHECKC(st.supply.symbol == sym,          err::SYMBOL_MISMATCH, "symbol precision mismatch");

    account_t::idx_t acnts(get_self(), owner.value);
    auto it = acnts.find(sym.code().raw());
    if (it == acnts.end()) {
        acnts.emplace(ram_payer, [&](auto& a){
            a.balance    = asset{0, sym};
            a.consumed   = asset{0, sym};
            a.created_at = current_time_point();
            a.updated_at = a.created_at;
        });
    }
}

void cisum_token::close( const name& owner, const symbol& symbol )
{
   require_auth( owner );
   account_t::idx_t acnts(get_self(), owner.value);
   auto it = acnts.find( symbol.code().raw() );
   check( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
   check( it->balance.amount == 0, "Cannot close because the balance is not zero." );
   acnts.erase( it );
}

void cisum_token::notifyaward(const name& user, const vector<nasset>& packs, const string& memo) {
    require_auth(get_self());
    if (_gstate.badgestore_contract.value != 0) {
      require_recipient(_gstate.badgestore_contract);
    }
}

static inline std::string build_award_memo(const eosio::name& user,
                                           const std::vector<nasset>& packs) {
    std::string prefix = "song.token|auto-award|" + user.to_string();
    std::string memo   = prefix;

    // 紧凑追加：|101x3|102x1 ...
    for (const auto& p : packs) {
        memo += "|" + std::to_string(p.symbol.nid) + "x" + std::to_string(p.amount);

        // 留点余量，超长就用总量兜底
        if (memo.size() >= 250) {
            int64_t total = 0;
            for (const auto& q : packs) total += q.amount;
            memo = prefix + "|amt=" + std::to_string(total);
            break;
        }
    }

    check(memo.size() <= 256, "memo too long");
    return memo;
}


void cisum_token::try_award_badges(const name& user,
                              int64_t consumed_before,
                              int64_t consumed_after)
{
    if (consumed_after <= consumed_before) return;

    badge_rule_t::idx_t rules(get_self(), get_self().value);
    if (rules.begin() == rules.end()) return;

    std::vector<nasset> packs;

    for (auto it = rules.begin(); it != rules.end(); ++it) {
        if (!it->enabled) continue;
        if (it->threshold.symbol != CISUM_SYM) continue;

        const int64_t step = it->threshold.amount;
        if (step <= 0) continue;

        const int64_t k_before = consumed_before / step;
        const int64_t k_after  = consumed_after  / step;
        const int64_t delta    = k_after - k_before;

        if (delta > 0) {
            packs.emplace_back(delta, it->symbol);
        }
    }

    if (packs.empty()) return;

    std::string memo = build_award_memo(user, packs);

    notifyaward_action{
        get_self(),
        { permission_level{ get_self(), "active"_n } }
    }.send(user, packs, memo);
}

} /// namespace eosio
