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
  ACTION init(const name& admin) ;

  /**
   * Create a new rush sale event.
   * Only admin can call this action.
   *
   * @param show_id              The show id.
   * @param ticket_id            The ticket id.
   * @param started_at           Sale start time.
   * @param ended_at             Sale end time.
   * @param price                Ticket price.
   * @param max_grabs_per_user   Max grabs per user.
   * @param win_ratio            Win ratio (1-10000).
   */
  ACTION addrushsale(const name&  submitter,
                                const  uint64_t&       show_id,
                                const  uint64_t&       ticket_id,
                                const  time_point&     started_at,
                                const  time_point&     ended_at,
                                const  asset&          price,
                                const  uint32_t&       max_grabs_per_user,
                                const  uint32_t&       win_ratio );

  /**
   * Delete a rush sale event.
   * Only admin can call this action.
   *
   * @param rush_sale_id         The rush sale id to delete.
   * @param forced               If true, force delete even if tickets sold.
   */
  ACTION delrushsale(const name& submitter,const uint64_t& rush_sale_id,const bool& forced );
  /**
   * Update rush sale parameters. Only admin can call.
   * Each parameter is optional and only updated if provided.
   *
   * @param rush_sale_id         The rush sale id to update.
   * @param win_ratio            Optional new win ratio.
   * @param ended_at             Optional new end time.
   */
  ACTION setrushsale(const name& submitter,
                      const uint64_t& rush_sale_id,
                      std::optional<uint32_t> max_grabs_per_user,
                      std::optional<uint32_t> win_ratio,
                      std::optional<time_point> ended_at) ;

  ACTION  settoken(const symbol& sym, const name& bank);

  ACTION  deltoken(const symbol& sym, const name& bank);

  ACTION clearsale(const name& submitter,const uint64_t& rush_sale_id) ;


  // 处理 FT（积分）转账：来自积分合约
  [[eosio::on_notify("*::transfer")]]
  void on_transfer(const name& from,
                        const name& to,
                        const asset& quantity,
                        const std::string& memo);

  // 处理 NFT 转账：来自票据合约
  [[eosio::on_notify("ticket.cvnft::transfer")]]
  void on_transfer_ticket(const name& from,
                          const name& to,
                          const std::vector<nasset>& assets,
                          const std::string& memo);

  /**
   * Notify user of ticket grab result.
   * Called internally after on_transfer.
   *
   * @param user                The user account.
   * @param rush_sale_id        The rush sale event id.
   * @param won                 True if user won the grab, false otherwise.
   */
  ACTION notifyticket(const std::string& grab_id,
                              const eosio::name& user,
                              uint32_t grabs,
                              const nasset& tickets,
                              const time_point& created_at
                              ,uint64_t rush_sale_id) ;

  /**
   * Configure the point contract infomation.
   * Only admin can call this action.
   * Only for test
   *
   * @param new_point_contract   The new point contract account name.
   */
  ACTION cfgpoint(const name& submitter,const name& new_point_contract);

  /**
   * Configure the ticket contract infomation.
   * Only admin can call this action.
   * Only for test
   *
   * @param new_ticket_contract   The new ticket contract account name.
   */
  ACTION cfgticket(const name& submitter,const name& new_ticket_contract) ;

  // -------- Inline wrappers --------
  using addrushsale_action        = eosio::action_wrapper<"addrushsale"_n,        &grab_cisum::addrushsale>;
  using delrushsale_action        = eosio::action_wrapper<"delrushsale"_n,        &grab_cisum::delrushsale>;
  using setrushsale_action        = eosio::action_wrapper<"setrushsale"_n,        &grab_cisum::setrushsale>;
  using notifyticket_action        = eosio::action_wrapper<"notifyticket"_n,        &grab_cisum::notifyticket>;

private:
  void require_role(const name& submitter,
                        const std::vector<std::string>& roles) const;

private:
  // 全局
  global_singleton _global;
  global_t         _gstate;

};

} // namespace flon