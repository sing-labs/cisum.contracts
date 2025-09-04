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
  ACTION init(const name& admin);

  // show_admin 管理
  ACTION addshowadm(const name& account);
  ACTION delshowadm(const name& account);

  // ✅ platform_admin 管理（新增）
  ACTION addplatadm(const name& account);
  ACTION delplatadm(const name& account);

  // per-show 核销员
  ACTION addchecker(const uint64_t& show_id, const name& account);
  ACTION delchecker(const uint64_t& show_id, const name& account);

  // === cvticket.nft: 创建票种 ===
  ACTION nftcreate(
                    const int64_t& max_supply,
                    const nsymbol& symbol,
                    const string&  token_uri);

  // === cvticket.nft: 发放（铸造到合约自身，再转出/或直接发放） ===
  ACTION nftissue(  const name&   issuer,
                const name&   to,
                const nasset& quantity,
                const string& memo);

  // ===== 演出 =====
  ACTION newshow(const uint64_t&   show_id,
                                  const name&       category,
                                  const bool&       ticket_transferable,
                                  const bool&       ticket_refundable,
                                  const time_point& show_started_at,
                                  const time_point& show_ended_at,
                                  const string&       show_name,
                                  const string&       show_address);

  ACTION setshow(const uint64_t&   show_id,
                                  const name&       category,
                                  const bool&       ticket_transferable,
                                  const bool&       ticket_refundable,
                                  const time_point& show_started_at,
                                  const time_point& show_ended_at,
                                  const string&       show_name,
                                  const string&       show_address);

  // ===== 票档 =====
  ACTION newticket(const uint64_t& show_id,
                                    const nsymbol&  ticket_nsym,
                                    const nsymbol&  prerequisite_nsym,
                                    const string&   ticket_type,
                                    const asset&    price,
                                    const asset&    price_usd,
                                    const time_point& sale_started_at,
                                    const time_point& sale_ended_at);

  ACTION setticket(const uint64_t& show_id,
                                    const uint64_t& ticket_id,
                                    const string&   ticket_type,
                                    const asset&    price,
                                    const asset&    price_usd,
                                    const time_point& sale_started_at,
                                    const time_point& sale_ended_at);

  // ===== 发放（从票档直接发 NFT）=====
  ACTION issue(const name&     user,
                               const uint64_t& show_id,
                               const uint64_t& ticket_id,
                               const uint32_t& ticket_count,
                               const string&   memo);


  ACTION issuetograb(const name& to, const nasset& quantity, const string& memo);

  using nftcreate_action     = eosio::action_wrapper<"nftcreate"_n,&show::nftcreate>;


  ACTION tkincrease(uint64_t               show_id,
                                  uint64_t               ticket_id,
                                  uint64_t                ticket_count,
                                  uint64_t                prev_ticket_count,
                                  const eosio::name&     issuer,
                                  const std::string&     memo,
                                  uint64_t               created_at
                                );

  using tkincrease_action     = eosio::action_wrapper<"tkincrease"_n,&show::tkincrease>;

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