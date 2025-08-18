#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include <cstring>
// #include <utils.hpp>
#include "grab.cisum.hpp"
#include <string>

namespace flon {

using namespace eosio;
using std::string;

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

void grab_cisum::addrushsale(   nsymbol        show_id,
                                nsymbol        ticket_id,
                                time_point     started_at,
                                time_point     ended_at,
                                asset          price,
                                uint32_t       max_grabs_per_user,
                                uint32_t       win_ratio,
                                uint32_t       total_tickets)
{
    require_auth(get_self());
    // TODO: check show_id valid?
    // TODO: check ticket_id valid?
    // check started_at < ended_at
    CHECKC(started_at < ended_at, err::INVALID_TIME, "started_at must be less than ended_at");
    CHECKC(price.symbol != NESTAR_SYMBOL, err::INVALID_FORMAT, "price symbol mismatch");
    CHECKC(price.amount > 0, err::INVALID_FORMAT, "price must be positive");
    // TODO: check price.symbol is valid?? price.symbol.is_valid()?

    CHECKC(win_ratio <= RATIO_BOOST, err::INVALID_FORMAT, "win_ratio can not larger than " + std::to_string(RATIO_BOOST));

    auto now = current_time_point();
    _gstate.last_rush_sale_id++;
    rush_sale::idx_t rs_idx = rush_sale::idx_t(get_self(), get_self().value);
    rs_idx.emplace(get_self(), [&](auto& rs){
        rs.id = _gstate.last_rush_sale_id;
        rs.show_id = show_id;
        rs.ticket_id = ticket_id;
        rs.started_at = started_at;
        rs.ended_at = ended_at;
        rs.price = price;
        rs.max_grabs_per_user = 1;
        rs.win_ratio = win_ratio;
        rs.total_tickets = total_tickets;
        rs.available_tickets = total_tickets;
        rs.sold_tickets = 0;
        rs.total_grabs = 0;
        rs.created_at = now;
        rs.updated_at = now;
    });
}

void grab_cisum::on_transfer( const name& from, const name& to, const asset& quantity, const string& memo) {
    if (from == get_self() || to != get_self()) return;
    require_auth( from );

    // memo format: "grab:${id}"
    auto memo_params = split(memo, ":");
    ASSERT( memo_params.size() > 1 )

    auto now = current_time_point();
    CHECKC( memo_params[0] == "grab",           err::INVALID_FORMAT,    "memo must start with 'grab'" )
    CHECKC (memo_params.size() >= 2,            err::INVALID_FORMAT,    "ontransfer: params size must lager than 2" )

    auto rush_sale_id       = std::stoul(string(memo_params[1]));
    rush_sale::idx_t rs_idx = rush_sale::idx_t(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC( rs_itr != rs_idx.end(), err::RECORD_NO_FOUND, "rush sale not found! id: " + std::to_string(rush_sale_id) )

    CHECKC( now >= rs_itr->started_at, err::STATUS_MISMATCH, "rush sale not started! id:" + std::to_string(rush_sale_id) )
    CHECKC( now <= rs_itr->ended_at, err::STATUS_MISMATCH, "rush sale ended! id:" + std::to_string(rush_sale_id) )
    CHECKC( rs_itr->available_tickets > 0, err::EXCEED_LIMIT, "rush sale has no available tickets" )
    ASSERT( rs_itr->total_tickets == rs_itr->available_tickets + rs_itr->sold_tickets)

    // TODO: only allow grab once at a time?
    CHECKC(quantity.symbol == rs_itr->price.symbol,    err::SYMBOL_MISMATCH,   "symbol mismatch");
    CHECKC( quantity == rs_itr->price, err::QUANTITY_MISMATCH, "quantity must be equal to rush sale price" )

    users::idx_t user_idx(get_self(), rush_sale_id);
    auto user_itr = user_idx.find(from.value);

    //TODO: user should open before grab tickets?
    if (user_itr == user_idx.end()) {
        user_itr = user_idx.emplace(get_self(), [&](auto& u){
            u.account = from;
        });
    }

    // check user can grab
    CHECKC( user_itr->grabs < rs_itr->max_grabs_per_user, err::EXCEED_LIMIT, "user's grabs exceeds max grabs limit of rush sale" )
    ASSERT( user_itr->tickets < rs_itr->total_tickets )

    // TODO: if user win, check by win rate, random number, then allocate tickets to user, and increase user_itr->tickets
    bool sold_tickets = 0;
    if (rs_itr->win_ratio > 0) {
        // Check if user wins using on-chain pseudo-random
        uint32_t random_number = get_random(from, RATIO_BOOST);
        if (random_number <= rs_itr->win_ratio) {
            sold_tickets = 1;
        }
    }

    ASSERT(sold_tickets <= 1);

    user_idx.modify(user_itr, same_payer, [&](auto& u) {
        u.grabs++;
        u.tickets += sold_tickets;
    });

    rs_idx.modify(rs_itr, same_payer, [&](auto& r) {
        r.total_grabs++;
        r.sold_tickets += sold_tickets;
        ASSERT(r.sold_tickets <= r.total_tickets);
        r.available_tickets = r.total_tickets - r.sold_tickets;
        r.updated_at = now;
    });
}

void grab_cisum::delrushsale( uint64_t rush_sale_id, bool forced ) {
    require_auth(get_self());

    rush_sale::idx_t rs_idx = rush_sale::idx_t(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC( rs_itr != rs_idx.end(), err::RECORD_NO_FOUND, "rush sale not found! id: " + std::to_string(rush_sale_id) )

    auto now = current_time_point();
    // If not forced, prevent deletion when there are grabs or sold tickets
    if (!forced) {
        bool is_grabbing = now >= rs_itr->started_at && now <= rs_itr->ended_at && rs_itr->available_tickets > 0;
        CHECKC( !is_grabbing, err::STATUS_MISMATCH, "rush sale is in the grabbing status, can not be deleted! id:" + std::to_string(rush_sale_id) );
    }

    rs_idx.erase(rs_itr);
}

void grab_cisum::delusers( uint64_t rush_sale_id, uint32_t max_count ) {
    require_auth(get_self());

    CHECKC( max_count > 0, err::NOT_POSITIVE, "max_count must be positive" );

    rush_sale::idx_t rs_idx = rush_sale::idx_t(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC( rs_itr == rs_idx.end(), err::NONE_DELETED, "rush sale must be deleted first" );

    users::idx_t user_idx(get_self(), rush_sale_id);
    auto user_itr = user_idx.begin();
    uint32_t count = 0;
    for (; count < max_count && user_itr != user_idx.end(); ) {
        user_itr = user_idx.erase(user_itr);
        count++;
    }
    CHECKC( count > 0, err::NONE_DELETED, "no users deleted" );
}

void grab_cisum::updrushsale(   uint64_t rush_sale_id,
                                std::optional<uint32_t> win_ratio,
                                std::optional<uint32_t> total_tickets,
                                std::optional<time_point> ended_at
  ) {
    require_auth(get_self());

    auto now = current_time_point();

    rush_sale::idx_t rs_idx = rush_sale::idx_t(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC( rs_itr != rs_idx.end(), err::RECORD_NO_FOUND, "rush sale not found! id: " + std::to_string(rush_sale_id) )

    // perform checks
    if (win_ratio.has_value()) {
        CHECKC(win_ratio.value() <= RATIO_BOOST, err::INVALID_FORMAT, "win_ratio can not larger than " + std::to_string(RATIO_BOOST));
    }
    if (total_tickets.has_value()) {
        CHECKC(total_tickets.value() >= rs_itr->sold_tickets, err::EXCEED_LIMIT, "total_tickets cannot be less than already sold tickets");
    }
    if (ended_at.has_value()) {
        CHECKC( rs_itr->started_at < ended_at.value(), err::INVALID_TIME, "ended_at must be greater than started_at");
        CHECKC( now < ended_at.value(), err::INVALID_TIME, "ended_at must be greater than current time");
    }

    rs_idx.modify(rs_itr, same_payer, [&](auto& r){
        if (win_ratio.has_value()) r.win_ratio = win_ratio.value();
        if (total_tickets.has_value()) {
            r.total_tickets = total_tickets.value();
            // adjust available tickets accordingly
            r.available_tickets = r.total_tickets - r.sold_tickets;
        }
        if (ended_at.has_value()) r.ended_at = ended_at.value();
        r.updated_at = now;
    });
}

} /// namespace flon
