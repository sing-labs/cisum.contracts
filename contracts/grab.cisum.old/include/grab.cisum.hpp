#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <optional>
#include <string>

#include "grab.cisum.db.hpp"

namespace flon {

using std::string;
using namespace eosio;
using std::vector;

class [[eosio::contract("grab.cisum")]] grab_cisum : public contract {
public:
    using contract::contract;

    grab_cisum(name receiver, name code, datastream<const char*> ds)
    : contract(receiver, code, ds),
      _global(get_self(), get_self().value)
    {
        _gstate = _global.exists() ? _global.get() : global_t{};
    }

    ~grab_cisum() {
        //_global.set(_gstate, get_self());
    }

    /**
     * Initialize the contract.
     * 权限：合约自身
     */
    ACTION init(const name& admin) ;

    ACTION delglobal();

    /**
     * Create a new rush sale event.
     * 权限：合约自身 / admin / OPS_CONTRACT / submitter(show)
     */
    ACTION addrushsale(const name& submitter,
                       const uint64_t& show_id,
                       const uint64_t& ticket_id,
                       const time_point& started_at,
                       const time_point& ended_at,
                       const asset& price,
                       const uint32_t& max_grabs_per_user,
                       const uint32_t& win_ratio);

    /**
     * Update rush sale parameters.
     * 权限：合约自身 / admin / OPS_CONTRACT / submitter(show)
     */
    ACTION setrushsale(const name& submitter,
                       const uint64_t& rush_sale_id,
                       std::optional<uint32_t> max_grabs_per_user,
                       std::optional<uint32_t> win_ratio,
                      std::optional<time_point> started_at,
                       std::optional<time_point> ended_at);

    /**
     * Clear orders and stats of a rush sale (after end).
     * 权限：合约自身 / admin / oracle
     */
    ACTION clearsale(const name& submitter, const uint64_t& rush_sale_id);


    ACTION delrushorder(const name& submitter, const uint64_t& rush_sale_id);

    /**
     * Delete a rush sale (optionally forced).
     * 权限：合约自身 / admin / oracle
     */
    ACTION delrushsale(const name& submitter,
                       const uint64_t& rush_sale_id,
                       const bool& forced);


    ACTION addupgrade(const name& submitter,
                            const uint64_t& show_id,
                            const uint64_t& target_ticket_id,
                            const nasset& pay_tickets,
                            const time_point& started_at,
                            const time_point& ended_at,
                            const uint32_t& win_ratio);
    ACTION setupgrade(const name& submitter,
                             const uint64_t& rush_upgrade_id,
                             std::optional<uint32_t> win_ratio,
                                std::optional<time_point> started_at,
                             std::optional<time_point> ended_at);

    ACTION delupgrade(const name& submitter,
                             const uint64_t& rush_upgrade_id,
                             const bool& forced);
    /**
     * Add or update allowed token.
     * 权限：合约自身 / admin
     */
    ACTION addtoken(const symbol& sym, const name& bank);

    ACTION addoracle(const name& account);
    ACTION deloracle(const name& account);


    /**
     * Delete allowed token.
     * 权限：合约自身 / admin
     */
    ACTION deltoken(const symbol& sym, const name& bank);

    /**
     * Internal notify for grab result.
     * 权限：合约自身
     */
    ACTION notifyticket(const std::string& grab_id,
                        const name& user,
                        uint32_t grabs,
                        const nasset& tickets,
                        const time_point& created_at,
                        uint64_t rush_sale_id);

    // -------- 配置 --------
    ACTION cfgpoint(const name& new_point_contract);
    ACTION cfgticket(const name& new_ticket_contract);

    // -------- on_notify --------
    [[eosio::on_notify("cisum.token::transfer")]]
    void on_transfer_cisum(const name& from,
                     const name& to,
                     const asset& quantity,
                     const std::string& memo);

    [[eosio::on_notify("ticket.cvnft::transfer")]]
    void on_transfer_ticket(const name& from,
                            const name& to,
                            const vector<nasset>& assets,
                            const std::string& memo);

    ACTION clearupgrade(const name& submitter, const uint64_t& rush_upgrade_id);

    // -------- inline wrappers --------
    using init_action         = action_wrapper<"init"_n,         &grab_cisum::init>;
    using addrushsale_action  = action_wrapper<"addrushsale"_n,  &grab_cisum::addrushsale>;
    using setrushsale_action  = action_wrapper<"setrushsale"_n,  &grab_cisum::setrushsale>;
    using clearsale_action    = action_wrapper<"clearsale"_n,    &grab_cisum::clearsale>;
    using delrushsale_action  = action_wrapper<"delrushsale"_n,  &grab_cisum::delrushsale>;
    using addtoken_action     = action_wrapper<"addtoken"_n,     &grab_cisum::addtoken>;
    using deltoken_action     = action_wrapper<"deltoken"_n,     &grab_cisum::deltoken>;
    using notifyticket_action = action_wrapper<"notifyticket"_n, &grab_cisum::notifyticket>;
    using cfgpoint_action     = action_wrapper<"cfgpoint"_n,     &grab_cisum::cfgpoint>;
    using cfgticket_action    = action_wrapper<"cfgticket"_n,    &grab_cisum::cfgticket>;
    using addupgrade_action   = action_wrapper<"addupgrade"_n,   &grab_cisum::addupgrade>;



private:
    void require_perm(const name& submitter, const std::string& perm) const;

    global_singleton _global;
    global_t         _gstate;

    void _process_add_rush_sale(const name& from,
                                const nasset& tickets,
                                const std::vector<std::string>& params);
    void _process_add_rush_upgrade(const name& from,
                                   const nasset& tickets,
                                   const std::vector<std::string>& params);
    void _process_rush_upgrade(const name& from,
                               const nasset& tickets,
                               const std::vector<std::string>& params);
};

} // namespace flon