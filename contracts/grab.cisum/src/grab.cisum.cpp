#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include "grab.cisum.hpp"
#include <string>
#include <flon/token.protocol.hpp>
#include "show.cisum.db.hpp"

namespace flon {

using namespace eosio;
using std::string;



// 将 16 字节数组转 32 位小写十六进制（只取前 16 字节 -> 32 hex）
static inline std::string to_hex32_from160_prefix(const checksum160& cs) {
    auto bytes = cs.extract_as_byte_array(); // 20 bytes
    static const char* HEX = "0123456789abcdef";
    std::string out;
    out.resize(32);                          // 16 bytes -> 32 hex chars

    // 只取前 16 字节，凑够 128bit（32 hex）
    size_t j = 0;
    for (size_t i = 0; i < 16; ++i) {
        uint8_t b = bytes[i];
        out[j++] = HEX[(b >> 4) & 0x0F];
        out[j++] = HEX[b & 0x0F];
    }
    return out;
}

// 生成 32 位十六进制ID： md5_like( 13位毫秒时间戳 + account )
// 说明：使用 ripemd160 代替 md5，并截取前 16 字节（128bit） => 32 hex
static inline std::string create_grab_id(const eosio::name& account) {
    // 13位毫秒级时间戳
    uint64_t ms = eosio::current_time_point().time_since_epoch().count() / 1000ULL;
    std::string payload = std::to_string(ms) + account.to_string();

    // ripemd160(payload) 并截取前 16 字节作为 32位hex
    checksum160 h = ripemd160(payload.data(), payload.size());
    return to_hex32_from160_prefix(h);
}


// TODO: move to utils.hpp of common lib
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

// generate a pseudo-random number in [1, range] using available on-chain entropy
static uint32_t get_random(const name& account, uint32_t range) {
    // Use transaction ID and current time for pseudo-randomness
    // Use tapos block prefix and current time, then XOR to form a seed
    uint32_t tapos = tapos_block_prefix();
    uint32_t timestamp = current_time_point().sec_since_epoch();
    uint64_t seed = uint64_t(tapos) ^ uint64_t(timestamp);

    uint64_t acc = account.value;
    uint32_t adata_size = action_data_size();

    // pack into a buffer: seed, account, action data size
    char buf[sizeof(seed) + sizeof(acc) + sizeof(adata_size)];
    size_t offset = 0;
    std::memcpy(buf + offset, &seed, sizeof(seed)); offset += sizeof(seed);
    std::memcpy(buf + offset, &acc, sizeof(acc)); offset += sizeof(acc);
    std::memcpy(buf + offset, &adata_size, sizeof(adata_size)); offset += sizeof(adata_size);

    checksum256 h = sha256(buf, offset);
    auto arr = h.extract_as_byte_array();
    uint32_t v = (uint32_t(arr[0]) << 24) | (uint32_t(arr[1]) << 16) | (uint32_t(arr[2]) << 8) | uint32_t(arr[3]);
    uint32_t r = (v % range) + 1;
    return r;
}


static inline uint32_t sha256_to_u32(const checksum256& d) {
    auto b = d.extract_as_byte_array();
    uint32_t v = 0;
    v |= (uint32_t)b[31] << 24;
    v |= (uint32_t)b[30] << 16;
    v |= (uint32_t)b[29] <<  8;
    v |= (uint32_t)b[28] <<  0;
    return v;
}

// 返回 [0, RATIO_BASE-1] 的随机数；salt 可用 rush_sale_id
static inline uint32_t get_random_base(const name& user, uint64_t salt) {
    uint64_t mix2 = (uint64_t)tapos_block_prefix();
    uint64_t mix3 = (uint64_t)tapos_block_num();

    std::array<char, 8*4> buf{};
    size_t o = 0;
    memcpy(buf.data()+o, &user.value, 8); o+=8;
    memcpy(buf.data()+o, &salt,       8); o+=8;
    memcpy(buf.data()+o, &mix2,       8); o+=8;
    memcpy(buf.data()+o, &mix3,       8); o+=8;

    auto h = sha256(buf.data(), o);
    return sha256_to_u32(h) % RATIO_BASE; // 0..9999
}



void grab_cisum::init(const name& admin) {
    require_auth(get_self());
    CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin must be a valid account");
    _gstate.admin = admin;
    _global.set(_gstate, get_self());
}

void grab_cisum::addrushsale( uint64_t       show_id,
                              uint64_t       ticket_id,
                              time_point     started_at,
                              time_point     ended_at,
                              asset          price,
                              uint32_t       max_grabs_per_user,
                              uint32_t       win_ratio )
{
    // ===== 权限：允许 admin / 本合约 / OPS_CONTRACT =====
    check(
        has_auth(_gstate.admin) ||
        has_auth(get_self())    ||
        has_auth(OPS_CONTRACT),
        "[[16]] requires admin, self, or cisumshowman auth"
    );

    // ===== 基础校验 =====
    CHECKC(ticket_id != 0,                  err::INVALID_FORMAT, "invalid ticket_id");
    CHECKC(started_at < ended_at,           err::INVALID_TIME,   "started_at must be less than ended_at");
    CHECKC(price.is_valid(),                err::INVALID_FORMAT, "invalid price asset");
    CHECKC(price.amount > 0,                err::INVALID_FORMAT, "price must be positive");
    CHECKC(max_grabs_per_user > 0,          err::NOT_POSITIVE,   "max_grabs_per_user must be positive");
    CHECKC(win_ratio <= RATIO_BASE,         err::INVALID_FORMAT, "win_ratio can not larger than " + std::to_string(RATIO_BASE));

    // ===== 校验 ticket 是否存在：作用域为 show_id =====
    {
        ticket_t::ticketidx tickets(SHOW_CONTRACT, show_id);
        auto tk_itr = tickets.find(ticket_id);
        CHECKC(tk_itr != tickets.end(), err::RECORD_NO_FOUND,
               "ticket_id not found in show contract: " + std::to_string(ticket_id));
    }

    // ===== 校验 price 的币种是否被允许（allowtokens 表）=====
    {
        allowed_token_t::idx_t tok(get_self(), get_self().value);
        auto bysym = tok.get_index<"bysymbol"_n>();
        uint128_t key = ( (uint128_t)price.symbol.code().raw() << 64 )
                      | (uint128_t)price.symbol.precision();
        auto itok = bysym.find(key);
        CHECKC(itok != bysym.end(), err::INVALID_FORMAT, "price symbol not allowed");

        CHECKC(itok->bank == _gstate.point_contract, err::INVALID_FORMAT, "price bank not allowed");
    }

    auto now = current_time_point();
    _gstate.last_rush_sale_id++;

    rush_sale::idx_t rs_idx(get_self(), get_self().value);
    rs_idx.emplace(get_self(), [&](auto& rs){
        rs.id                  = _gstate.last_rush_sale_id;
        rs.show_id             = show_id;
        rs.ticket_id           = ticket_id;
        rs.started_at          = started_at;
        rs.ended_at            = ended_at;
        rs.price               = price;
        rs.max_grabs_per_user  = max_grabs_per_user;
        rs.win_ratio           = win_ratio;
        rs.total_tickets       = nasset(0, nsymbol(ticket_id));
        rs.available_tickets   = nasset(0, nsymbol(ticket_id));
        rs.sold_tickets        = nasset(0, nsymbol(ticket_id));
        rs.total_grabs         = 0;
        rs.created_at          = now;
        rs.updated_at          = now;
    });
}

void grab_cisum::settoken(const symbol& sym, const name& bank) {
    require_auth(get_self());

    check(sym.is_valid(),                "invalid symbol");
    check(sym.precision() <= 8,          "precision too large");
    check(is_account(bank),              "bank account not exist");

    allowed_token_t::idx_t tbl(get_self(), get_self().value);
    auto bysym = tbl.get_index<"bysymbol"_n>();

    const uint128_t key = ( (uint128_t)sym.code().raw() << 64 ) | (uint128_t)sym.precision();
    auto it = bysym.find(key);

    const auto now = current_time_point();

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
            r.bank       = bank;   // 更新 bank
            r.updated_at = now;
        });
    }
}

void grab_cisum::deltoken(const symbol& sym, const name& bank) {
    require_auth(get_self());

    check(sym.is_valid(),   "invalid symbol");
    check(is_account(bank), "bank account not exist");

    allowed_token_t::idx_t tbl(get_self(), get_self().value);
    auto bysym = tbl.get_index<"bysymbol"_n>();

    const uint128_t key = ( (uint128_t)sym.code().raw() << 64 ) | (uint128_t)sym.precision();
    auto it = bysym.find(key);
    check(it != bysym.end(), "token not found");

    // 需要同时匹配 bank 才能删，避免误删同 code/precision 但 bank 不同的历史记录
    check(it->bank == bank, "bank mismatch");

    bysym.erase(it);
}

void grab_cisum::on_transfer_point(const name& from,
                                   const name& to,
                                   const asset& quantity,
                                   const string& memo) {
    if (from == get_self() || to != get_self()) return;

    // memo: "grab:<rush_sale_id>:<grab_id>"
    auto params = split(memo, ":");
    CHECKC(params.size() > 2, err::INVALID_FORMAT, "memo must be grab:<sale_id>:<grab_id>");
    CHECKC(params[0] == "grab", err::INVALID_FORMAT, "memo must start with 'grab'");
    CHECKC(!params[1].empty() && !params[2].empty(), err::INVALID_FORMAT, "sale_id or grab_id cannot be empty");

    const uint64_t    rush_sale_id = std::stoull(params[1]);
    const std::string grab_id      = params[2];
    auto              now          = current_time_point();

    // 查找 rush sale
    rush_sale::idx_t rs_idx(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC(rs_itr != rs_idx.end(), err::RECORD_NO_FOUND, "rush sale not found");
    CHECKC(now >= rs_itr->started_at, err::STATUS_MISMATCH, "rush sale not started");
    CHECKC(now <= rs_itr->ended_at,   err::STATUS_MISMATCH, "rush sale ended");
    CHECKC(rs_itr->available_tickets.amount > 0, err::EXCEED_LIMIT, "no tickets left");
    ASSERT(rs_itr->total_tickets == rs_itr->available_tickets + rs_itr->sold_tickets);

    CHECKC(quantity.symbol == rs_itr->price.symbol, err::SYMBOL_MISMATCH, "symbol mismatch");
    CHECKC(quantity == rs_itr->price, err::QUANTITY_MISMATCH, "quantity must equal price");
    CHECKC(rs_itr->win_ratio <= RATIO_BASE, err::EXCEED_LIMIT, "win_ratio must be in 0..10000");

    // 抽签
    bool win = false;
    if (rs_itr->win_ratio > 0) {
        uint32_t rnd = get_random_base(from, rush_sale_id); // 0..9999
        win = (rnd < rs_itr->win_ratio);
    }

    if (win) {
        // 中奖才写入 orders 表
        order_t::idx_t orders(get_self(), rush_sale_id);

        // 用 grab_id 二级索引判重
        auto h      = sha256(grab_id.data(), grab_id.size());
        auto bygrab = orders.template get_index<"bygrabid"_n>();
        CHECKC(bygrab.find(h) == bygrab.end(), err::TYPE_INVALID, "duplicate grab_id");

        orders.emplace(get_self(), [&](auto& o){
            o.id         = orders.available_primary_key();
            o.grab_id    = grab_id;
            o.account    = from;
            o.grabs      = 1; // 固定写 1
            o.tickets    = nasset(1, nsymbol(rs_itr->ticket_id));
            o.created_at = now;
        });

        // 发 NFT
        std::vector<nasset> assets = { nasset(1, nsymbol(rs_itr->ticket_id)) };
        TRANSFER_NFT_OUT(_gstate.ticket_contract, from, assets, "grab ticket");

        // 更新场次
        rs_idx.modify(rs_itr, same_payer, [&](auto& r){
            r.sold_tickets.amount += 1;
            ASSERT(r.sold_tickets.is_amount_within_range());
            ASSERT(r.sold_tickets <= r.total_tickets);
            r.available_tickets = r.total_tickets - r.sold_tickets;
            r.total_grabs++;
            r.updated_at = now;
        });
    } else {
        // 未中奖：只累计总参与次数
        rs_idx.modify(rs_itr, same_payer, [&](auto& r){
            r.total_grabs++;
            r.updated_at = now;
        });
    }

    // 无论中不中都发通知
    grab_cisum::notifyticket_action act{ get_self(), { {get_self(), "active"_n} } };
    nasset result_ticket = win ? nasset(1, nsymbol(rs_itr->ticket_id))
                            : nasset(0, nsymbol(rs_itr->ticket_id));

    act.send(grab_id, from, 1, result_ticket, now,rush_sale_id);
}

void grab_cisum::on_transfer_ticket( const name& from, const name& to, const vector<nasset>& assets, const string& memo ) {
    if ( from == get_self() || to != get_self()) return;
    // memo format: "add:${rush_sale_id}"
    auto memo_params = split(memo, ":");
    ASSERT( memo_params.size() > 1 )

    auto now = current_time_point();
    CHECKC( memo_params[0] == "add",           err::INVALID_FORMAT,    "memo must start with 'add'" )
    CHECKC (memo_params.size() == 2,           err::INVALID_FORMAT,    "ontransfer: params size must be equal to 2" )
    CHECKC (assets.size() == 1,                err::INVALID_FORMAT,    "ontransfer: nasset count must be equal to 1" )

    auto rush_sale_id       = std::stoul(string(memo_params[1]));
    rush_sale::idx_t rs_idx = rush_sale::idx_t(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC( rs_itr != rs_idx.end(), err::RECORD_NO_FOUND, "rush sale not found! id: " + std::to_string(rush_sale_id) )

    const auto& tickets = assets[0];
    // TODO: only allow grab once at a time?
    CHECKC(tickets.symbol == rs_itr->total_tickets.symbol, err::SYMBOL_MISMATCH,
            "ticket symbol mismatch, rush_sale_id=" + std::to_string(rush_sale_id));
    CHECKC( tickets.amount > 0, err::NOT_POSITIVE, "must transfer positive amount" );

    rs_idx.modify(rs_itr, same_payer, [&](auto& r) {
        r.total_tickets += tickets;
        ASSERT(r.sold_tickets <= r.total_tickets);
        r.available_tickets = r.total_tickets - r.sold_tickets;
        r.updated_at = now;
    });
}

void grab_cisum::notifyticket(const std::string& grab_id,
                              const eosio::name& user,
                              uint32_t grabs,
                              const nasset& tickets,
                              const time_point& created_at
                              ,uint64_t rush_sale_id) {
    require_auth(get_self());
    require_recipient(user);
}

void grab_cisum::delrushsale( uint64_t rush_sale_id, bool forced ) {
    require_auth(_gstate.admin);

    rush_sale::idx_t rs_idx = rush_sale::idx_t(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC( rs_itr != rs_idx.end(), err::RECORD_NO_FOUND, "rush sale not found! id: " + std::to_string(rush_sale_id) )

    auto now = current_time_point();
    // If not forced, prevent deletion when there are grabs or sold tickets
    if (!forced) {
        bool is_grabbing = now >= rs_itr->started_at && now <= rs_itr->ended_at && rs_itr->available_tickets.amount > 0;
        CHECKC( !is_grabbing, err::STATUS_MISMATCH, "rush sale is in the grabbing status, can not be deleted! id:" + std::to_string(rush_sale_id) );
    }

    // TODO: how to process the remaining tickets in the rush sale??

    rs_idx.erase(rs_itr);
}

void grab_cisum::delusers(const uint64_t rush_sale_id, const uint32_t max_count) {
    require_auth(_gstate.admin);
    CHECKC(max_count > 0, err::NOT_POSITIVE, "max_count must be positive");

    // 活动必须先被删除（即查不到）才允许清 orders 表
    rush_sale::idx_t rs_idx(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC(rs_itr == rs_idx.end(), err::NONE_DELETED, "rush sale must be deleted first");

    order_t::idx_t orders(get_self(), rush_sale_id);
    if (orders.begin() == orders.end()) {
        CHECKC(false, err::NONE_DELETED, "no orders to delete");
    }

    uint32_t count = 0;
    for (auto it = orders.begin(); count < max_count && it != orders.end(); ) {
        it = orders.erase(it);
        ++count;
    }

    CHECKC(count > 0, err::NONE_DELETED, "no orders deleted");
}

void grab_cisum::setrushsale(
    uint64_t rush_sale_id,
    std::optional<uint32_t> max_grabs_per_user,
    std::optional<uint32_t> win_ratio,
    std::optional<time_point> ended_at
) {
    require_auth(_gstate.admin);
    auto now = current_time_point();

    rush_sale::idx_t rs_idx(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC(rs_itr != rs_idx.end(), err::RECORD_NO_FOUND,
           "rush sale not found! id: " + std::to_string(rush_sale_id));

    if (max_grabs_per_user.has_value()) {
        CHECKC(max_grabs_per_user.value() > 0, err::NOT_POSITIVE,
               "max_grabs_per_user must be positive");
    }
    if (win_ratio.has_value()) {
        CHECKC(win_ratio.value() <= RATIO_BASE, err::INVALID_FORMAT,
               "win_ratio can not larger than " + std::to_string(RATIO_BASE));
    }
    if (ended_at.has_value()) {
        CHECKC(rs_itr->started_at < ended_at.value(), err::INVALID_TIME,
               "ended_at must be greater than started_at");
        CHECKC(now < ended_at.value(), err::INVALID_TIME,
               "ended_at must be greater than current time");
    }

    rs_idx.modify(rs_itr, same_payer, [&](auto& r){
        if (max_grabs_per_user.has_value()) r.max_grabs_per_user = max_grabs_per_user.value();
        if (win_ratio.has_value())          r.win_ratio = win_ratio.value();
        if (ended_at.has_value())           r.ended_at = ended_at.value();
        r.updated_at = now;
    });
}

void grab_cisum::cfgpoint(const eosio::name& new_point_contract) {
    require_auth(_gstate.admin);
    CHECKC(is_account(new_point_contract), err::ACCOUNT_INVALID, "point_contract must be a valid account");
    _gstate.point_contract = new_point_contract;
}

void grab_cisum::cfgticket(const eosio::name& new_ticket_contract) {
    require_auth(_gstate.admin);
    CHECKC(is_account(new_ticket_contract), err::ACCOUNT_INVALID, "ticket_contract must be a valid account");
    _gstate.ticket_contract = new_ticket_contract;
}

} /// namespace flon