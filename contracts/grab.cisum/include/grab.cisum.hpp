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

  [[eosio::action]]
  void init(const eosio::name& admin);

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

  [[eosio::action]]
  void delrushsale( uint64_t rush_sale_id, bool forced );

  [[eosio::action]]
  void updrushsale( uint64_t rush_sale_id,
                    std::optional<uint32_t> win_ratio,
                    std::optional<uint32_t> total_tickets,
                    std::optional<time_point> ended_at
  );

  [[eosio::action]]
  void delusers( uint64_t rush_sale_id, uint32_t max_count );

  [[eosio::on_notify("nestar.cisum::transfer")]]
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