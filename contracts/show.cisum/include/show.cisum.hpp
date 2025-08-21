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
  [[eosio::action]] void init(const name& admin);

  // show_admin 管理
  [[eosio::action]] void addshowadm(const name& account);
  [[eosio::action]] void delshowadm(const name& account);

  // ✅ platform_admin 管理（新增）
  [[eosio::action]] void addplatadm(const name& account);
  [[eosio::action]] void delplatadm(const name& account);

  // per-show 核销员
  [[eosio::action]] void addchecker(const uint64_t& show_id, const name& account);
  [[eosio::action]] void delchecker(const uint64_t& show_id, const name& account);

  // === cvticket.nft: 创建票种 ===
  [[eosio::action]]
  void nftcreate(const int64_t&  max_supply,
                const nsymbol&  symbol,
                const string&   token_uri);

  // === cvticket.nft: 发放（铸造到合约自身，再转出/或直接发放） ===
  [[eosio::action]]
  void nftissue(const nasset&  quantity,
               const string&  memo);

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

  // ===== 发放（从票档直接发 NFT）=====
  [[eosio::action]] void issue(const name&     user,
                               const uint64_t& show_id,
                               const uint64_t& ticket_id,
                               const uint32_t& amount,
                               const string&   memo);

private:
  void require_platform_admin() const;
  void require_show_admin() const;
  void require_admin_or_showadm() const;
  void require_admin_or_platadm() const;
  void require_any_admin() const; // admin OR platform_admin OR show_admin

private:
  global_singleton _global;
  global_t         _gstate;
};

} // namespace flon