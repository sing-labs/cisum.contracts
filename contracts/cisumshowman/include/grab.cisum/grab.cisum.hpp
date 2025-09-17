#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <optional>
#include <string>

#include "grab.cisum.db.hpp"



#define ADDRUSHSALE(bank,submitter, show_id,ticket_id,started_at,ended_at,price,max_grabs_per_user,win_ratio) \
    {	flon::grab_cisum::addrushsale_action act{ bank, { {submitter, "active"_n} } };\
			act.send(submitter, show_id,ticket_id,started_at,ended_at,price,max_grabs_per_user,win_ratio);}

namespace flon {

using std::string;
using namespace eosio;
using std::vector;

class [[eosio::contract("grab.cisum")]] grab_cisum : public contract {
public:
  using contract::contract;

  ACTION init(const eosio::name& admin);

  ACTION addrushsale(const name&  submitter,
                    const  uint64_t&       show_id,
                    const  uint64_t&       ticket_id,
                    const  time_point&     started_at,
                    const  time_point&     ended_at,
                    const  asset&          price,
                    const  uint32_t&       max_grabs_per_user,
                    const  uint32_t&       win_ratio );

  ACTION delrushsale(const name& submitter,
                    const uint64_t& rush_sale_id,
                    const bool& forced );
  ACTION setrushsale(const name& submitter,
                      const uint64_t& rush_sale_id,
                      std::optional<uint32_t> max_grabs_per_user,
                      std::optional<uint32_t> win_ratio,
                      std::optional<time_point> ended_at) ;

  ACTION notifyticket(const std::string& grab_id,
                              const eosio::name& user,
                              uint32_t grabs,
                              const nasset& tickets,
                              const time_point& created_at
                              ,uint64_t rush_sale_id) ;

  ACTION cfgpoint(const name& submitter,const name& new_point_contract);


  ACTION cfgticket(const name& submitter,const name& new_ticket_contract) ;


  // -------- Inline wrappers --------
  using addrushsale_action        = eosio::action_wrapper<"addrushsale"_n,        &grab_cisum::addrushsale>;
  using delrushsale_action        = eosio::action_wrapper<"delrushsale"_n,        &grab_cisum::delrushsale>;
  using cfgrushsale_action        = eosio::action_wrapper<"setrushsale"_n,        &grab_cisum::setrushsale>;
  using notifyticket_action       = eosio::action_wrapper<"notifyticket"_n,        &grab_cisum::notifyticket>;

};

} // namespace flon