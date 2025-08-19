#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <optional>
#include <string>

#include "grab.cisum.db.hpp"

using std::string;
using namespace eosio;

namespace flon {

class [[eosio::contract("grab.cisum")]] grab_cisum : public contract {
public:
  using contract::contract;

  grab_cisum(eosio::name receiver, eosio::name code, datastream<const char*> ds)
  : contract(receiver, code, ds),
    _global(get_self(), get_self().value)
  {
    _gstate = _global.exists() ? _global.get() : global_t{};
  }

  ~grab_cisum() { _global.set(_gstate, get_self()); }

  /**
   * Initialize the contract and set the admin account.
   * Only contract self can call this action.
   *
   * @param admin                The account to be set as admin.
   */
  [[eosio::action]]
  void init(const eosio::name& admin);

  /**
   * Create a new rush sale event.
   * Only admin can call this action.
   *
   * @param show_id              The show identifier.
   * @param ticket_id            The ticket identifier.
   * @param started_at           Sale start time.
   * @param ended_at             Sale end time.
   * @param price                Ticket price.
   * @param max_grabs_per_user   Max grabs per user.
   * @param win_ratio            Win ratio (1-10000).
   * @param total_tickets        Total tickets available.
   */
  [[eosio::action]]
  void addrushsale(   nsymbol        show_id,
                      nsymbol        ticket_id,
                      time_point     started_at,
                      time_point     ended_at,
                      asset          price,
                      uint32_t       max_grabs_per_user,
                      uint32_t       win_ratio,
                      uint32_t       total_tickets
   );

  /**
   * Delete a rush sale event.
   * Only admin can call this action.
   *
   * @param rush_sale_id         The rush sale id to delete.
   * @param forced               If true, force delete even if tickets sold.
   */
  [[eosio::action]]
  void delrushsale( uint64_t rush_sale_id, bool forced );
  /**
   * Update rush sale parameters. Only admin can call.
   * Each parameter is optional and only updated if provided.
   *
   * @param rush_sale_id         The rush sale id to update.
   * @param win_ratio            Optional new win ratio.
   * @param max_grabs_per_user   Optional new max grabs per user.
   * @param total_tickets        Optional new total tickets.
   * @param ended_at             Optional new end time.
   */
  [[eosio::action]]
  void updrushsale( uint64_t                  rush_sale_id,
                    std::optional<uint32_t>   win_ratio,
                    std::optional<uint32_t>   total_tickets,
                    std::optional<time_point> ended_at
  );

  /**
   * Batch delete users for a rush sale. Only admin can call.
   *
   * @param rush_sale_id         The rush sale id.
   * @param max_count            Max number of users to delete in one call.
   */
  [[eosio::action]]
  void delusers( uint64_t rush_sale_id, uint32_t max_count );

  /**
   * Handle incoming token transfer for grabbing tickets.
   * Called automatically on transfer from POINT_TOKEN.
   *
   * @param from                 Sender account.
   * @param to                   Receiver (contract self).
   * @param quantity             Amount transferred.
   * @param memo                 Memo string, format: "grab:<rush_sale_id>".
   */
  [[eosio::on_notify("*::transfer")]]
  void on_transfer(const name& from, const name& to, const asset& quantity, const string& memo);

  // -------- Inline wrappers --------
  using addrushsale_action        = eosio::action_wrapper<"addrushsale"_n,        &grab_cisum::addrushsale>;
  using delrushsale_action        = eosio::action_wrapper<"delrushsale"_n,        &grab_cisum::delrushsale>;
  using updrushsale_action        = eosio::action_wrapper<"updrushsale"_n,        &grab_cisum::updrushsale>;

private:
  // 全局
  global_singleton _global;
  global_t         _gstate;

};

} // namespace flon