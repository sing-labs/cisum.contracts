#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <string>

#include <show.cisum.db.hpp>

namespace flon {

using namespace eosio;
using std::string;
using flon::nsymbol;

class [[eosio::contract("show.cisum")]] show : public contract {
public:
  using contract::contract;

  show(name receiver, name code, datastream<const char*> ds)
  : contract(receiver, code, ds),
    _global(get_self(), get_self().value) {
    _gstate = _global.exists() ? _global.get() : global_t{};
  }
  ~show() { _global.set(_gstate, get_self()); }

  // ===== 全局设置 =====
  [[eosio::action]] void init(const name& admin, const name& nft_bank);
  [[eosio::action]] void setcvticket(const name& nft_bank);
  [[eosio::action]] void addshowadm(const name& account);
  [[eosio::action]] void delshowadm(const name& account);

  [[eosio::action]]
  void addchecker(const uint64_t& show_id, const name& account);

  [[eosio::action]]
  void delchecker(const uint64_t& show_id, const name& account);


  // ===== 演出 =====
  [[eosio::action]] void newshow(const name&       category,
                                 const bool&       ticket_transferable,
                                 const bool&       ticket_refundable,
                                 const time_point& sale_started_at,
                                 const time_point& sale_ended_at,
                                 const time_point& show_started_at,
                                 const time_point& show_ended_at,
                                 const name&       status);

  [[eosio::action]] void setshow(const uint64_t&   show_id,
                                 const name&       category,
                                 const bool&       ticket_transferable,
                                 const bool&       ticket_refundable,
                                 const time_point& sale_started_at,
                                 const time_point& sale_ended_at,
                                 const time_point& show_started_at,
                                 const time_point& show_ended_at,
                                 const name&       status);

  [[eosio::action]] void showstatus(const uint64_t& show_id,
                                    const name&     status);

  // ===== 票档 =====
  [[eosio::action]] void newticket(const uint64_t& show_id,
                                   const nsymbol&  ticket_nsym,
                                   const nsymbol&  prerequisite_nsym,
                                   const string&   ticket_type,
                                   const asset&    price,
                                   const uint32_t& total_count,
                                   const name&     status);

  [[eosio::action]] void setticket(const uint64_t& show_id,
                                   const uint64_t& ticket_id,
                                   const string&   ticket_type,
                                   const asset&    price,
                                   const uint32_t& total_count,
                                   const name&     status);

  [[eosio::action]] void ticketstatus(const uint64_t& show_id,
                                      const uint64_t& ticket_id,
                                      const name&     status);

  // ===== 发放 =====
  [[eosio::action]] void issue(const name&     user,
                               const uint64_t& show_id,
                               const uint64_t& ticket_id,
                               const uint32_t& amount,
                               const string&   memo);

private:
  global_singleton _global;
  global_t         _gstate;

  void check_showadm() const;

  void require_issue_auth(uint64_t show_id) const;
};

} // namespace flon