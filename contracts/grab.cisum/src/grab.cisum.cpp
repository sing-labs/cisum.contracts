#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
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

void grab_cisum::addrushsale( nsymbol        show_id,
                        nsymbol        ticket_id,
                        time_point     started_at,
                        time_point     ended_at,
                        asset          price,
                        uint32_t       max_grabs_per_user,
                        uint32_t       win_ratio,
                        uint32_t       total_tickets)
{
    require_auth(get_self());
    // TODO: ...
    // CHECKC(is_account(issuer), err::ACCOUNT_INVALID, "issuer account not exist");
    // _gstate.issuer = issuer;
}

void grab_cisum::on_transfer( const name& from, const name& to, const nasset& quantity, const string& memo) {
    if (from == get_self() || to != get_self()) return;
    require_auth( from );

    // memo format: "grab:${id}"
    auto memo_params = split(memo, ":");
    ASSERT( memo_params.size() > 1 )

    auto now = current_time_point();
    CHECKC( memo_params[0] == "grab",           err::INVALID_FORMAT,    "memo must start with 'grab'" )
    CHECKC (memo_params.size() == 2,            err::INVALID_FORMAT,    "ontransfer: params size must be equal to 2" )
    // CHECKC(quantity.symbol == NESTAR_SYMBOL,    err::SYMBOL_MISMATCH,   "symbol or precision mismatch");

    auto rush_sale_id       = std::stoul(string(memo_params[1]));
    rush_sale::idx_t rs_idx = rush_sale::idx_t(get_self(), rush_sale_id);
    auto rs_itr = rs_idx.find(rush_sale_id);
    CHECKC( rs_itr != rs_idx.end(), err::RECORD_NO_FOUND, "rush sale not found! id: " + std::to_string(rush_sale_id) )

    CHECKC( now >= rs_itr->started_at, err::STATUS_MISMATCH, "rush sale not started! id:" + std::to_string(rush_sale_id) )
    CHECKC( now <= rs_itr->ended_at, err::STATUS_MISMATCH, "rush sale ended! id:" + std::to_string(rush_sale_id) )
    CHECKC( rs_itr->available_tickets > 0, err::EXCEED_LIMIT, "rush sale has no available tickets" )
    ASSERT( rs_itr->total_tickets == rs_itr->available_tickets + rs_itr->sold_tickets)

    // TODO: only allow grab once at a time?
    // CHECKC( quantity == rs_itr.price, err::QUANTITY_MISMATCH, "quantity must be equal to rush sale price" )

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
        // Check if user wins
        // uint32_t random_number = eosio::random::get_random_number(1, 10000);
        // if (random_number <= rs_itr->win_ratio) {
        //     sold_tickets = 1;
        // }
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
    });
}


} /// namespace flon
