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
#include <flon/token.protocol.hpp>

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

void grab_cisum::init(const name& admin) {
    require_auth(get_self());
    CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin must be a valid account");
    _gstate.admin = admin;
    // _gstate saved in ~grab_cisum()
}

// add to show.cisum contract
// addrushsale(nsymbol        show_id,
//                                 nsymbol        ticket_id,
//                                 time_point     started_at,
//                                 time_point     ended_at,
//                                 asset          price,
//                                 uint32_t       max_grabs_per_user,
//                                 uint32_t       win_ratio,
//                             int64_t tickets) {
//     rush_sale_id = grab.cisum.global.last_sale_id + 1;
//     addrushsale_action.send()
//     transfer_action.send(tickets)
// }

void grab_cisum::addrushsale(   uint64_t       show_id,
                                uint64_t       ticket_id,
                                time_point     started_at,
                                time_point     ended_at,
                                asset          price,
                                uint32_t       max_grabs_per_user,
                                uint32_t       win_ratio)
{
    require_auth(_gstate.admin);
    // TODO: check show_id valid?
    // TODO: check ticket_id valid?
    CHECKC(ticket_id != 0, err::INVALID_FORMAT, "invalid ticket_id");
    // check started_at < ended_at
    CHECKC(started_at < ended_at, err::INVALID_TIME, "started_at must be less than ended_at");
    CHECKC(price.symbol == POINT_SYMBOL, err::INVALID_FORMAT, "price symbol mismatch");
    CHECKC(price.amount > 0, err::INVALID_FORMAT, "price must be positive");
    // TODO: check price.symbol is valid?? price.symbol.is_valid()?

    CHECKC(win_ratio <= RATIO_BOOST, err::INVALID_FORMAT, "win_ratio can not larger than " + std::to_string(RATIO_BOOST));


    auto now = current_time_point();
    _gstate.last_rush_sale_id++;
    rush_sale::idx_t rs_idx = rush_sale::idx_t(get_self(), get_self().value);
    rs_idx.emplace(get_self(), [&](auto& rs){
        rs.id = _gstate.last_rush_sale_id;
        rs.show_id              = show_id;
        rs.ticket_id            = ticket_id;
        rs.started_at           = started_at;
        rs.ended_at             = ended_at;
        rs.price                = price;
        rs.max_grabs_per_user   = 1;
        rs.win_ratio            = win_ratio;
        rs.total_tickets        = nasset(0, nsymbol(ticket_id));
        rs.available_tickets    = nasset(0, nsymbol(ticket_id));
        rs.sold_tickets         = nasset(0, nsymbol(ticket_id));
        rs.total_grabs          = 0;
        rs.created_at           = now;
        rs.updated_at           = now;
    });
}

void grab_cisum::on_transfer() {
    if (get_first_receiver() == _gstate.point_contract) {
        execute_action(*this, &grab_cisum::on_transfer_point);
    } else if (get_first_receiver() == _gstate.ticket_contract) {
        execute_action(*this, &grab_cisum::on_transfer_ticket);
    }
}

void grab_cisum::on_transfer_point( const name& from, const name& to, const asset& quantity, const string& memo) {
    if ( from == get_self() || to != get_self()) return;

    // TODO: add nonce param to memo
    // memo format: "grab:${rush_sale_id}"
    auto memo_params = split(memo, ":");
    ASSERT( memo_params.size() > 1 )

    auto now = current_time_point();
    CHECKC( memo_params[0] == "grab",           err::INVALID_FORMAT,    "memo must start with 'grab'" )
    CHECKC (memo_params.size() > 1,            err::INVALID_FORMAT,    "ontransfer: params size must be larger than 1" )

    auto rush_sale_id       = std::stoul(string(memo_params[1]));
    rush_sale::idx_t rs_idx = rush_sale::idx_t(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC( rs_itr != rs_idx.end(), err::RECORD_NO_FOUND, "rush sale not found! id: " + std::to_string(rush_sale_id) )

    CHECKC( now >= rs_itr->started_at, err::STATUS_MISMATCH, "rush sale not started! id:" + std::to_string(rush_sale_id) )
    CHECKC( now <= rs_itr->ended_at, err::STATUS_MISMATCH, "rush sale ended! id:" + std::to_string(rush_sale_id) )
    CHECKC( rs_itr->available_tickets.amount > 0, err::EXCEED_LIMIT, "rush sale has no available tickets" )
    ASSERT( rs_itr->total_tickets == rs_itr->available_tickets + rs_itr->sold_tickets)

    // TODO: only allow grab once at a time?
    CHECKC(quantity.symbol == rs_itr->price.symbol,    err::SYMBOL_MISMATCH,   "symbol mismatch");
    CHECKC( quantity == rs_itr->price, err::QUANTITY_MISMATCH, "quantity must be equal to rush sale price" )

    user_t::idx_t user_idx(get_self(), rush_sale_id);
    auto user_itr = user_idx.find(from.value);

    //TODO: user should open before grab tickets?
    if (user_itr == user_idx.end()) {
        user_itr = user_idx.emplace(get_self(), [&](auto& u){
            u.account = from;
            u.tickets = nasset(0, nsymbol(rs_itr->ticket_id));
        });
    }

    // check user can grab
    CHECKC( user_itr->grabs < rs_itr->max_grabs_per_user, err::EXCEED_LIMIT, "user's grabs exceeds max grabs limit of rush sale" )
    ASSERT( user_itr->tickets < rs_itr->total_tickets )

    bool win = false;
    if (rs_itr->win_ratio > 0) {
        // Check if user wins using on-chain pseudo-random
        uint32_t random_number = get_random(from, RATIO_BOOST);
        win = random_number <= rs_itr->win_ratio;
        std::vector<nasset> assets = {nasset(1, nsymbol(rs_itr->ticket_id))};
        TRANSFER_NFT_OUT(_gstate.ticket_contract, from, assets, "grab ticket");
    }

    user_idx.modify(user_itr, same_payer, [&](auto& u) {
        u.grabs++;
        if (win) u.tickets.amount += 1;
        assert(u.tickets.is_amount_within_range());
    });

    rs_idx.modify(rs_itr, same_payer, [&](auto& r) {
        r.total_grabs++;
        if (win) r.sold_tickets.amount += 1;
        ASSERT(r.sold_tickets.is_amount_within_range())
        ASSERT(r.sold_tickets <= r.total_tickets);
        r.available_tickets = r.total_tickets - r.sold_tickets;
        r.updated_at = now;
    });

    // Notify the user of the grab result
    grab_cisum::notifyticket_action act{ get_self(), { {get_self(), "active"_n} } };
    act.send( from, rush_sale_id, win );
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

void grab_cisum::notifyticket(const eosio::name& user, uint64_t rush_sale_id, bool won) {
    require_auth(get_self());
    require_recipient( user );
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

void grab_cisum::delusers( uint64_t rush_sale_id, uint32_t max_count ) {
    require_auth(_gstate.admin);

    CHECKC( max_count > 0, err::NOT_POSITIVE, "max_count must be positive" );

    rush_sale::idx_t rs_idx = rush_sale::idx_t(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC( rs_itr == rs_idx.end(), err::NONE_DELETED, "rush sale must be deleted first" );

    user_t::idx_t user_idx(get_self(), rush_sale_id);
    auto user_itr = user_idx.begin();
    uint32_t count = 0;
    for (; count < max_count && user_itr != user_idx.end(); ) {
        user_itr = user_idx.erase(user_itr);
        count++;
    }
    CHECKC( count > 0, err::NONE_DELETED, "no users deleted" );
}

void grab_cisum::cfgrushsale(   uint64_t rush_sale_id,
                                std::optional<uint32_t> win_ratio,
                                std::optional<time_point> ended_at
    ) {
        require_auth(_gstate.admin);
    auto now = current_time_point();

    rush_sale::idx_t rs_idx = rush_sale::idx_t(get_self(), get_self().value);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC( rs_itr != rs_idx.end(), err::RECORD_NO_FOUND, "rush sale not found! id: " + std::to_string(rush_sale_id) )

    // perform checks
    if (win_ratio.has_value()) {
        CHECKC(win_ratio.value() <= RATIO_BOOST, err::INVALID_FORMAT, "win_ratio can not larger than " + std::to_string(RATIO_BOOST));
    }
    if (ended_at.has_value()) {
        CHECKC( rs_itr->started_at < ended_at.value(), err::INVALID_TIME, "ended_at must be greater than started_at");
        CHECKC( now < ended_at.value(), err::INVALID_TIME, "ended_at must be greater than current time");
    }

    rs_idx.modify(rs_itr, same_payer, [&](auto& r){
        if (win_ratio.has_value()) r.win_ratio = win_ratio.value();
        if (ended_at.has_value()) r.ended_at = ended_at.value();
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
