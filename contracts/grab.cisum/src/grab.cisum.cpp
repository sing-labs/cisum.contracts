#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>

#include "grab.cisum.hpp"
#include <flon/token.protocol.hpp>
#include "show.cisum.db.hpp"
#include "flon.auth/flon.auth.hpp"

namespace flon {

using namespace eosio;
using std::string;


// 将 16 字节数组转 32 位小写十六进制（只取前 16 字节 -> 32 hex）
static inline std::string to_hex32_from160_prefix(const checksum160& cs) {
    auto bytes = cs.extract_as_byte_array();
    static const char* HEX = "0123456789abcdef";
    std::string out;
    out.resize(32);

    size_t j = 0;
    for (size_t i = 0; i < 16; ++i) {
        uint8_t b = bytes[i];
        out[j++] = HEX[(b >> 4) & 0x0F];
        out[j++] = HEX[b & 0x0F];
    }
    return out;
}

// 生成 32 位十六进制ID： ripemd160(毫秒时间戳 + account) 前16字节
static inline std::string create_grab_id(const eosio::name& account) {
    uint64_t ms = eosio::current_time_point().time_since_epoch().count() / 1000ULL;
    std::string payload = std::to_string(ms) + account.to_string();
    checksum160 h = ripemd160(payload.data(), payload.size());
    return to_hex32_from160_prefix(h);
}

// 字符串切割
std::vector<std::string> split(const std::string& s, const std::string& delimiter) {
    std::vector<std::string> result;
    size_t pos_start = 0, pos_end;
    auto delim_len = delimiter.length();
    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        result.emplace_back(s.substr(pos_start, pos_end - pos_start));
        pos_start = pos_end + delim_len;
    }
    result.emplace_back(s.substr(pos_start));
    return result;
}

// sha256 转 uint32
static inline uint32_t sha256_to_u32(const checksum256& d) {
    auto b = d.extract_as_byte_array();
    uint32_t v = 0;
    v |= (uint32_t)b[31] << 24;
    v |= (uint32_t)b[30] << 16;
    v |= (uint32_t)b[29] << 8;
    v |= (uint32_t)b[28];
    return v;
}

// 基础随机数生成
static uint32_t get_random(const name& account, uint32_t range) {
    uint32_t tapos = tapos_block_prefix();
    uint32_t timestamp = current_time_point().sec_since_epoch();
    uint64_t seed = uint64_t(tapos) ^ uint64_t(timestamp);

    uint64_t acc = account.value;
    uint32_t adata_size = action_data_size();

    char buf[sizeof(seed) + sizeof(acc) + sizeof(adata_size)];
    size_t offset = 0;
    memcpy(buf + offset, &seed, sizeof(seed)); offset += sizeof(seed);
    memcpy(buf + offset, &acc, sizeof(acc)); offset += sizeof(acc);
    memcpy(buf + offset, &adata_size, sizeof(adata_size)); offset += sizeof(adata_size);

    checksum256 h = sha256(buf, offset);
    auto arr = h.extract_as_byte_array();
    uint32_t v = (uint32_t(arr[0]) << 24) | (uint32_t(arr[1]) << 16) |
                 (uint32_t(arr[2]) << 8)  | (uint32_t(arr[3]));
    return (v % range) + 1;
}

// 返回 [0, RATIO_BASE-1] 的随机数；salt 用 rush_sale_id
static inline uint32_t get_random_base(const name& user, uint64_t salt) {
    uint32_t tapos = tapos_block_prefix();
    uint32_t timestamp = current_time_point().sec_since_epoch();
    uint64_t seed = uint64_t(tapos) ^ uint64_t(timestamp) ^ salt;

    uint64_t acc = user.value;
    uint32_t adata_size = action_data_size();

    char buf[sizeof(seed) + sizeof(acc) + sizeof(adata_size)];
    size_t offset = 0;
    memcpy(buf + offset, &seed, sizeof(seed)); offset += sizeof(seed);
    memcpy(buf + offset, &acc, sizeof(acc)); offset += sizeof(acc);
    memcpy(buf + offset, &adata_size, sizeof(adata_size)); offset += sizeof(adata_size);

    checksum256 h = sha256(buf, offset);
    auto arr = h.extract_as_byte_array();
    uint32_t v = (uint32_t(arr[0]) << 24) | (uint32_t(arr[1]) << 16) |
                 (uint32_t(arr[2]) << 8)  | (uint32_t(arr[3]));
    return v % RATIO_BASE;
}

// ======================================================
// 权限校验
// ======================================================

void grab_cisum::require_perm(const name& submitter, const std::string& perm) const {
    require_auth(submitter);
    flonauth::checkrole_action(
        CISUMAUTH_CONTRACT,
        { get_self(), "active"_n }
    ).send(get_self(), submitter, perm);
}

void grab_cisum::init(const name& admin) {
    require_auth(get_self());
    CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin must be a valid account");
    _gstate.admin = admin;
    _global.set(_gstate, get_self());
}

void grab_cisum::cfgpoint(const name& new_point_contract) {
    check(has_auth(get_self()) || has_auth(_gstate.admin),
          "requires self or admin auth");
    CHECKC(is_account(new_point_contract), err::ACCOUNT_INVALID, "point_contract must be valid");
    _gstate.point_contract = new_point_contract;
}

void grab_cisum::cfgticket(const name& new_ticket_contract) {
    check(has_auth(get_self()) || has_auth(_gstate.admin),
          "requires self or admin auth");
    CHECKC(is_account(new_ticket_contract), err::ACCOUNT_INVALID, "ticket_contract must be valid");
    _gstate.ticket_contract = new_ticket_contract;
}

void grab_cisum::addtoken(const symbol& sym, const name& bank) {
    check(has_auth(get_self()) || has_auth(_gstate.admin),
          "requires self or admin auth");

    check(sym.is_valid(), "invalid symbol");
    check(sym.precision() <= 8, "precision too large");
    check(is_account(bank), "bank account not exist");

    allowed_token_t::idx_t tbl(get_self(), get_self().value);
    auto bysym = tbl.get_index<"bysymbol"_n>();
    uint128_t key = ((uint128_t)sym.code().raw() << 64) | sym.precision();

    auto it = bysym.find(key);
    auto now = current_time_point();
    if (it == bysym.end()) {
        tbl.emplace(get_self(), [&](auto& r){
            r.id         = tbl.available_primary_key();
            r.sym        = sym;
            r.bank       = bank;
            r.created_at = now;
            r.updated_at = now;
        });
    } else {
        bysym.modify(it, same_payer, [&](auto& r){
            r.bank       = bank;
            r.updated_at = now;
        });
    }
}

void grab_cisum::deltoken(const symbol& sym, const name& bank) {
    check(has_auth(get_self()) || has_auth(_gstate.admin),
          "requires self or admin auth");

    check(sym.is_valid(), "invalid symbol");
    check(is_account(bank), "bank account not exist");

    allowed_token_t::idx_t tbl(get_self(), get_self().value);
    auto bysym = tbl.get_index<"bysymbol"_n>();
    uint128_t key = ((uint128_t)sym.code().raw() << 64) | sym.precision();

    auto it = bysym.find(key);
    check(it != bysym.end(), "token not found");
    check(it->bank == bank, "bank mismatch");
    bysym.erase(it);
}

void grab_cisum::addoracle(const name& account) {
    require_auth(get_self());

    CHECKC(account.value != 0,  err::ACCOUNT_INVALID, "oracle account cannot be empty");
    CHECKC(is_account(account), err::ACCOUNT_INVALID, "oracle account not exist");

    _gstate.oracles.insert(account);
    _global.set(_gstate, get_self());
}

void grab_cisum::deloracle(const name&  account) {
    require_auth(get_self());

    CHECKC(account.value != 0,  err::ACCOUNT_INVALID, "oracle account cannot be empty");

    auto it = _gstate.oracles.find(account);
    if (it != _gstate.oracles.end()) {
        _gstate.oracles.erase(it);
        _global.set(_gstate, get_self());
    }
}


// ======================================================
// 核心业务逻辑
// ======================================================

void grab_cisum::addrushsale(const name& submitter,
                             const uint64_t& show_id,
                             const uint64_t& ticket_id,
                             const time_point& started_at,
                             const time_point& ended_at,
                             const asset& price,
                             const uint32_t& max_grabs_per_user,
                             const uint32_t& win_ratio) {
    bool authed = has_auth(get_self()) || has_auth(OPS_CONTRACT) || has_auth(_gstate.admin);
    if (!authed) {
        require_perm(submitter, "show");
        authed = true;
    }
    check(authed, "requires self/ops/admin OR submitter with perm=show");

    CHECKC(ticket_id != 0, err::INVALID_FORMAT, "invalid ticket_id");
    CHECKC(started_at < ended_at, err::INVALID_TIME, "started_at must be less than ended_at");
    CHECKC(price.is_valid(), err::INVALID_FORMAT, "invalid price asset");
    CHECKC(price.amount > 0, err::INVALID_FORMAT, "price must be positive");
    CHECKC(max_grabs_per_user > 0, err::NOT_POSITIVE, "max_grabs_per_user must be positive");
    CHECKC(win_ratio <= RATIO_BASE, err::INVALID_FORMAT, "win_ratio too large");

    {   // 检查 ticket 是否存在
        ticket_t::ticketidx tickets(SHOW_CONTRACT, show_id);
        auto tk_itr = tickets.find(ticket_id);
        CHECKC(tk_itr != tickets.end(), err::RECORD_NO_FOUND,
               "ticket_id not found in show contract");
    }

    {   // 检查 price 是否在 allowtokens 中
        allowed_token_t::idx_t tok(get_self(), get_self().value);
        auto bysym = tok.get_index<"bysymbol"_n>();
        uint128_t key = ((uint128_t)price.symbol.code().raw() << 64) | price.symbol.precision();
        auto itok = bysym.find(key);
        CHECKC(itok != bysym.end(), err::INVALID_FORMAT,
               "price symbol not allowed: " + price.symbol.code().to_string());
    }

    auto now = current_time_point();
    _gstate.last_rush_sale_id++;

    rush_sale::idx_t rs_idx(get_self(), get_self().value);
    rs_idx.emplace(get_self(), [&](auto& rs){
        rs.id                 = _gstate.last_rush_sale_id;
        rs.show_id            = show_id;
        rs.ticket_id          = ticket_id;
        rs.started_at         = started_at;
        rs.ended_at           = ended_at;
        rs.price              = price;
        rs.max_grabs_per_user = max_grabs_per_user;
        rs.win_ratio          = win_ratio;
        rs.total_tickets      = nasset(0, nsymbol(ticket_id));
        rs.available_tickets  = nasset(0, nsymbol(ticket_id));
        rs.grabbed_tickets       = nasset(0, nsymbol(ticket_id));
        rs.total_grabs        = 0;
        rs.created_at         = now;
        rs.updated_at         = now;
    });
}

void grab_cisum::setrushsale(const name& submitter,
                             const uint64_t& rush_sale_id,
                             std::optional<uint32_t> max_grabs_per_user,
                             std::optional<uint32_t> win_ratio,
                             std::optional<time_point> ended_at) {
    bool authed = has_auth(get_self()) || has_auth(OPS_CONTRACT) || has_auth(_gstate.admin);
    if (!authed) {
        require_perm(submitter, "show");
        authed = true;
    }
    check(authed, "requires self/ops/admin OR submitter with perm=show");

    auto now = current_time_point();
    rush_sale::idx_t rs_idx(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC(rs_itr != rs_idx.end(), err::RECORD_NO_FOUND, "rush sale not found");

    if (max_grabs_per_user.has_value())
        CHECKC(max_grabs_per_user.value() > 0, err::NOT_POSITIVE, "max_grabs_per_user must be positive");
    if (win_ratio.has_value())
        CHECKC(win_ratio.value() <= RATIO_BASE, err::INVALID_FORMAT, "win_ratio too large");
    if (ended_at.has_value())
        CHECKC(rs_itr->started_at < ended_at.value(), err::INVALID_TIME, "ended_at must > started_at");

    rs_idx.modify(rs_itr, same_payer, [&](auto& r){
        if (max_grabs_per_user) r.max_grabs_per_user = *max_grabs_per_user;
        if (win_ratio)          r.win_ratio = *win_ratio;
        if (ended_at)           r.ended_at  = *ended_at;
        r.updated_at = now;
    });
}

void grab_cisum::clearsale(const name& submitter, const uint64_t& rush_sale_id) {
    if (!(has_auth(get_self()) || has_auth(_gstate.admin))) {
        check(_gstate.oracles.find(submitter) != _gstate.oracles.end(),
              "requires self, admin, or oracle auth");
        require_auth(submitter);
    }

    auto now = current_time_point();
    rush_sale::idx_t rs_idx(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC(rs_itr != rs_idx.end(), err::RECORD_NO_FOUND, "rush sale not found");
    CHECKC(now > rs_itr->ended_at, err::STATUS_MISMATCH, "rush sale not ended yet");

    {   // 清理 orders
        order_t::idx_t orders(get_self(), rush_sale_id);
        for (auto itr = orders.begin(); itr != orders.end(); )
            itr = orders.erase(itr);
    }

    {   // 清理 stats
        grab_stat_t::idx_t stats(get_self(), rush_sale_id);
        for (auto itr = stats.begin(); itr != stats.end(); )
            itr = stats.erase(itr);
    }
}

void grab_cisum::delrushsale(const name& submitter,
                             const uint64_t& rush_sale_id,
                             const bool& forced) {
    if (!(has_auth(get_self()) || has_auth(_gstate.admin))) {
        check(_gstate.oracles.find(submitter) != _gstate.oracles.end(),
              "requires self, admin, or oracle auth");
        require_auth(submitter);
    }

    rush_sale::idx_t rs_idx(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC(rs_itr != rs_idx.end(), err::RECORD_NO_FOUND, "rush sale not found");

    auto now = current_time_point();
    if (!forced) {
        bool is_grabbing = now >= rs_itr->started_at && now <= rs_itr->ended_at &&
                           rs_itr->available_tickets.amount > 0;
        CHECKC(!is_grabbing, err::STATUS_MISMATCH, "rush sale still active, cannot delete");
    }

    rs_idx.erase(rs_itr);
}

void grab_cisum::addupgrade(const name& submitter,
                            const uint64_t& show_id,
                            const uint64_t& target_ticket_id,
                            const nasset& pay_tickets,
                            const time_point& started_at,
                            const time_point& ended_at,
                            const uint32_t& win_ratio)
{
    // 1) 权限
    bool authed = has_auth(get_self()) || has_auth(OPS_CONTRACT) || has_auth(_gstate.admin);
    if (!authed) {
        require_perm(submitter, "show");
        authed = true;
    }
    check(authed, "requires self/ops/admin OR submitter with perm=show");

    // 2) 参数校验

    CHECKC(started_at < ended_at, err::INVALID_TIME, "started_at must be less than ended_at");
    CHECKC(pay_tickets.amount > 0, err::INVALID_FORMAT, "pay_tickets must be positive");
    CHECKC(pay_tickets.is_valid(), err::INVALID_FORMAT, "invalid pay_tickets (symbol/amount)");
    CHECKC(win_ratio <= RATIO_BASE, err::INVALID_FORMAT, "win_ratio too large");

    // 3) 主票 ticket 是否存在
    ticket_t::ticketidx tickets(SHOW_CONTRACT, show_id);
    auto tk_itr = tickets.find(target_ticket_id);
    CHECKC(tk_itr != tickets.end(), err::RECORD_NO_FOUND,
           "ticket_id not found in show contract");

    // 4) 校验 pay_tickets 的 NFT 合法性
    //    在 flon::nsymbol 中，id() 返回的是低 9 位的 NFT id（与你们 ticket_id 对应）
    const uint64_t pay_ticket_id = pay_tickets.symbol.value;

    // 4.1 不允许费用 NFT 和目标 ticket 是同一个 id
    CHECKC(pay_ticket_id != target_ticket_id, err::INVALID_FORMAT,
           "(pay ticket)  must NOT equal target_ticket_id");

    // 4.2 费用 NFT 必须存在于 tickets（按你需求）
    {
        auto pay_itr = tickets.find(pay_ticket_id);
        CHECKC(pay_itr != tickets.end(), err::RECORD_NO_FOUND,
               " (pay ticket) not found in show contract");
    }

    // 5) 写入 rush_upgrade 表
    auto now = current_time_point();
    _upggstate.last_rush_upgrade_id++; // 确保 _global_state 有此字段

    rush_upgrade::idx_t ru_idx(get_self(), get_self().value);
    auto ticket                 = nasset(0, nsymbol(target_ticket_id));
    ru_idx.emplace(get_self(), [&](auto& ru){
        ru.id                   = _upggstate.last_rush_upgrade_id;
        ru.show_id              = show_id;
        ru.target_ticket_id     = target_ticket_id;
        ru.pay_tickets          = pay_tickets;
        ru.started_at           = started_at;
        ru.ended_at             = ended_at;
        ru.win_ratio            = win_ratio;
        ru.total_tickets        = ticket;
        ru.available_tickets    = ticket;
        ru.grabbed_tickets      = ticket;
        ru.total_grabs          = 0;
        ru.created_at           = now;
        ru.updated_at           = now;
    });
}

void grab_cisum::setupgrade(const name& submitter,
                             const uint64_t& rush_upgrade_id,
                             std::optional<uint32_t> win_ratio,
                             std::optional<time_point> ended_at) {
    bool authed = has_auth(get_self()) || has_auth(OPS_CONTRACT) || has_auth(_gstate.admin);
    if (!authed) {
        require_perm(submitter, "show");
        authed = true;
    }
    check(authed, "requires self/ops/admin OR submitter with perm=show");

    auto now = current_time_point();
    rush_upgrade::idx_t ru_idx(get_self(), get_self().value);
    auto ru_itr = ru_idx.find(rush_upgrade_id);
    CHECKC(ru_itr != ru_idx.end(), err::RECORD_NO_FOUND, "rush upgrade not found");

    if (win_ratio.has_value())
        CHECKC(win_ratio.value() <= RATIO_BASE, err::INVALID_FORMAT, "win_ratio too large");
    if (ended_at.has_value())
        CHECKC(ru_itr->started_at < ended_at.value(), err::INVALID_TIME, "ended_at must > started_at");

    ru_idx.modify(ru_itr, same_payer, [&](auto& r){
        if (win_ratio)          r.win_ratio = *win_ratio;
        if (ended_at)           r.ended_at  = *ended_at;
        r.updated_at = now;
    });
}



void grab_cisum::on_transfer_cisum(const name& from,
                                    const name& to,
                                    const asset& quantity,
                                    const string& memo) {
    if (from == get_self() || to != get_self()) return;

    // 校验 token
    allowed_token_t::idx_t tokens(get_self(), get_self().value);
    auto bysym = tokens.get_index<"bysymbol"_n>();
    uint128_t key = ((uint128_t)quantity.symbol.code().raw() << 64) | quantity.symbol.precision();
    auto itok = bysym.find(key);
    CHECKC(itok != bysym.end(), err::INVALID_FORMAT, "token not allowed");
    CHECKC(itok->bank == get_first_receiver(), err::DID_NOT_AUTH, "transfer not from correct bank");

    auto params = split(memo, ":");
    CHECKC(params.size() > 2 && params[0] == "grab", err::INVALID_FORMAT, "memo must be grab:<sale_id>:<grab_id>");

    uint64_t rush_sale_id = std::stoull(params[1]);
    string grab_id = params[2];
    auto now = current_time_point();

    rush_sale::idx_t rs_idx(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC(rs_itr != rs_idx.end(), err::RECORD_NO_FOUND, "rush sale not found");
    CHECKC(now >= rs_itr->started_at, err::STATUS_MISMATCH, "rush sale not started");
    CHECKC(now <= rs_itr->ended_at, err::STATUS_MISMATCH, "rush sale ended");
    CHECKC(rs_itr->available_tickets.amount > 0, err::EXCEED_LIMIT, "no tickets left");

    CHECKC(quantity.symbol == rs_itr->price.symbol, err::SYMBOL_MISMATCH, "symbol mismatch");
    CHECKC(quantity == rs_itr->price, err::QUANTITY_MISMATCH, "quantity must equal price");

    order_t::idx_t orders(get_self(), rush_sale_id);
    auto bygrab = orders.get_index<"bygrabid"_n>();
    auto h = sha256(grab_id.data(), grab_id.size());
    CHECKC(bygrab.find(h) == bygrab.end(), err::TYPE_INVALID, "duplicate grab_id");

    grab_stat_t::idx_t stats(get_self(), rush_sale_id);
    auto st = stats.find(from.value);
    uint32_t used = (st == stats.end()) ? 0 : st->grabs;
    if (rs_itr->max_grabs_per_user > 0)
        CHECKC(used < rs_itr->max_grabs_per_user, err::EXCEED_LIMIT, "exceed max grabs per user");

    // 防止同一用户重复中奖
    uint128_t key_userwin = ((uint128_t)from.value << 1) | 1;
    auto byuw = orders.get_index<"byuserwin"_n>();
    CHECKC(byuw.find(key_userwin) == byuw.end(), err::EXCEED_LIMIT, "user already won a ticket in this rush sale");

    // 抽签
    bool win = false;
    if (rs_itr->win_ratio > 0) {
        uint32_t rnd = get_random_base(from, rush_sale_id);
        win = (rnd < rs_itr->win_ratio);
    }

    // 记录 order
    orders.emplace(get_self(), [&](auto& o){
        o.id         = orders.available_primary_key();
        o.grab_id    = grab_id;
        o.account    = from;
        o.grabs      = 1;
        o.tickets    = win ? nasset(1, nsymbol(rs_itr->ticket_id)) : nasset(0, nsymbol(rs_itr->ticket_id));
        o.created_at = now;
    });

    if (st == stats.end()) {
        stats.emplace(get_self(), [&](auto& s){
            s.account    = from;
            s.grabs      = 1;
            s.updated_at = now;
        });
    } else {
        stats.modify(st, same_payer, [&](auto& s){
            s.grabs += 1;
            s.updated_at = now;
        });
    }

    if (win) {
        std::vector<nasset> assets = { nasset(1, nsymbol(rs_itr->ticket_id)) };
        TRANSFER_NFT_OUT(_gstate.ticket_contract, from, assets, "grab ticket");
        rs_idx.modify(rs_itr, same_payer, [&](auto& r){
            r.grabbed_tickets.amount += 1;
            r.available_tickets = r.total_tickets - r.grabbed_tickets;
            r.total_grabs++;
            r.updated_at = now;
        });
    } else {
        rs_idx.modify(rs_itr, same_payer, [&](auto& r){
            r.total_grabs++;
            r.updated_at = now;
        });
    }

    grab_cisum::notifyticket_action act{ get_self(), { {get_self(), "active"_n} } };
    nasset result_ticket = win ? nasset(1, nsymbol(rs_itr->ticket_id)) : nasset(0, nsymbol(rs_itr->ticket_id));
    act.send(grab_id, from, 1, result_ticket, now, rush_sale_id);
}

/* Purpose: pay_ticket -> target_ticket */
void grab_cisum::on_transfer_ticket(const name& from,
                                    const name& to,
                                    const vector<nasset>& assets,
                                    const string& memo)
{
    if (from == get_self() || to != get_self()) return;
    CHECKC(!assets.empty(), err::INVALID_FORMAT, "no assets transferred");

    const auto& tickets = assets[0];
    auto params = split(memo, ":");
    CHECKC(params.size() >= 2, err::INVALID_FORMAT, "invalid memo");

    std::string action = params[0];
    if (action == "addrushsale") {
        _process_add_rush_sale(from, tickets, params); return;
    } else if (action == "addrushupgrade") {
        _process_add_rush_upgrade(from, tickets, params); return;
    } else if (action == "rushupgrade") {
        _process_rush_upgrade(from, tickets, params); return;
    }
            
    CHECKC(false, err::INVALID_FORMAT, "invalid action in memo");
}

void grab_cisum::_process_add_rush_sale(const name& from,
                                        const nasset& tickets,
                                        const std::vector<std::string>& params)
 {
// 管理员添加库存 (addrushsale:<rush_sale_id>)

    CHECKC(params.size() == 2, err::INVALID_FORMAT, "memo must be addrushsale:<rush_sale_id>");
    uint64_t rush_sale_id = std::stoull(params[1]);

    rush_sale::idx_t rs_idx(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC(rs_itr != rs_idx.end(), err::RECORD_NO_FOUND, "rush sale not found");

    CHECKC(tickets.symbol == rs_itr->total_tickets.symbol, err::SYMBOL_MISMATCH, "ticket symbol mismatch");
    CHECKC(tickets.amount > 0, err::NOT_POSITIVE, "must transfer positive amount");

    auto now = current_time_point();
    rs_idx.modify(rs_itr, same_payer, [&](auto& r){
        r.total_tickets += tickets;
        r.available_tickets = r.total_tickets - r.grabbed_tickets;
        r.updated_at = now;
    });
}

// 管理员添加 rush_upgrade 库存
void grab_cisum::_process_add_rush_upgrade(const name& from,
                                           const nasset& tickets,
                                           const std::vector<std::string>& params)
{
    // memo = "addrushupgrade:<rush_upgrade_id>"
    CHECKC(params.size() == 2, err::INVALID_FORMAT, "memo must be addrushupgrade:<rush_upgrade_id>");
    uint64_t rush_upgrade_id = std::stoull(params[1]);

    rush_upgrade::idx_t ru_idx(get_self(), get_self().value);
    auto ru_itr = ru_idx.find(rush_upgrade_id);
    CHECKC(ru_itr != ru_idx.end(), err::RECORD_NO_FOUND, "rush upgrade not found");

    CHECKC(tickets.symbol == ru_itr->total_tickets.symbol, err::SYMBOL_MISMATCH, "ticket symbol mismatch");
    CHECKC(tickets.amount > 0, err::NOT_POSITIVE, "must transfer positive amount");

    auto now = current_time_point();
    ru_idx.modify(ru_itr, same_payer, [&](auto& r){
        r.total_tickets += tickets;
        r.available_tickets = r.total_tickets - r.grabbed_tickets;
        r.updated_at = now;
    });
}

void grab_cisum::_process_rush_upgrade(const name& from,
                                       const nasset& tickets,
                                       const std::vector<std::string>& params)
{
    // 用户 rush 升级活动参与（rushupgrade:<rush_upgrade_id>:<grab_id>）

    CHECKC(params.size() == 3, err::INVALID_FORMAT, "memo must be rushupgrade:<rush_upgrade_id>:<grab_id>");
    uint64_t rush_upgrade_id = std::stoull(params[1]);
    std::string grab_id = params[2];
    auto now = current_time_point();

    // 加载活动
    rush_upgrade::idx_t ru_idx(get_self(), get_self().value);
    auto ru_itr = ru_idx.find(rush_upgrade_id);
    CHECKC(ru_itr != ru_idx.end(), err::RECORD_NO_FOUND, "rush upgrade not found");
    CHECKC(now >= ru_itr->started_at, err::STATUS_MISMATCH, "rush upgrade not started");
    CHECKC(now <= ru_itr->ended_at, err::STATUS_MISMATCH, "rush upgrade ended");
    CHECKC(ru_itr->available_tickets.amount > 0, err::EXCEED_LIMIT, "no tickets left");

    // 校验符号一致
    CHECKC(tickets == ru_itr->pay_tickets, err::SYMBOL_MISMATCH, "pay tickets requirement mismatch");

    // 防重复参与（grab_id 唯一）
    upgrade_log_t::idx_t logs(get_self(), rush_upgrade_id);
    auto bygrab = logs.get_index<"bygrabid"_n>();
    auto h = sha256(grab_id.data(), grab_id.size());
    CHECKC(bygrab.find(h) == bygrab.end(), err::TYPE_INVALID, "duplicate grab_id");

    // 防止同一用户重复中奖
    uint128_t key_userwin = ((uint128_t)from.value << 1) | 1;
    auto byuw = logs.get_index<"byuserwin"_n>();
    CHECKC(byuw.find(key_userwin) == byuw.end(), err::EXCEED_LIMIT, "user already won in this upgrade");

    // 抽签判定
    bool win = false;
    if (ru_itr->win_ratio > 0) {
        uint32_t rnd = get_random_base(from, rush_upgrade_id);
        win = (rnd < ru_itr->win_ratio);
    }
    
    auto num = win ? 1 : 0;
    auto to_ticket = nasset( num , nsymbol(ru_itr->target_ticket_id) );

    // 更新 rush_upgrade 状态
    ru_idx.modify(ru_itr, same_payer, [&](auto& r){
        r.total_grabs++;
        if (win) {
            r.grabbed_tickets.amount++;
            r.available_tickets = r.total_tickets - r.grabbed_tickets;
        }
        r.updated_at = now;
    });

    // 写入日志
    logs.emplace(get_self(), [&](auto& row){
        row.id         = logs.available_primary_key();
        row.grab_id    = grab_id;
        row.account    = from;
        row.from       = tickets; // from: 消耗票, to: 升级结果
        row.to         = to_ticket;
        row.created_at = now;
    });

    // 发奖
    if (win) {
        // 发放升级票
        vector<nasset> assets_out = { to_ticket };
        TRANSFER_NFT_OUT(_gstate.ticket_contract, from, assets_out, "rushupgrade reward");
    }
}

void grab_cisum::notifyticket(const string& grab_id,
                              const name& user,
                              uint32_t grabs,
                              const nasset& tickets,
                              const time_point& created_at,
                              uint64_t rush_sale_id) {
    require_auth(get_self());
    require_recipient(user);
}




} // namespace flon