#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <string>

namespace flon {

using namespace eosio;
using std::string;
using flon::nsymbol;

#define CREATE_NFT(bank,max_supply, symbol, token_uri) \
    {	flon::show::nftcreate_action act{ bank, { {get_self(), "active"_n} } };\
			act.send( max_supply, symbol, token_uri);}

#define ISSUE_NFT(bank,issuer,quantity,memo) \
    {	flon::show::nftissue_action act{ bank, { {get_self(), "active"_n} } };\
			act.send( issuer,quantity,memo);}

#define NEW_SHOW(bank,show_id,category,ticket_transferable,ticket_refundable,show_started_at,show_ended_at,show_name,show_address) \
    {	flon::show::newshow_action act{ bank, { {get_self(), "active"_n} } };\
			act.send( show_id,category,ticket_transferable,ticket_refundable,show_started_at,show_ended_at,show_name,show_address);}

#define NEW_TICKET(bank,show_id,ticket_nsym,prerequisite_nsym,ticket_type,price,price_usdt,sale_started_at,sale_ended_at) \
    {	flon::show::newticket_action act{ bank, { {get_self(), "active"_n} } };\
			act.send( show_id,ticket_nsym,prerequisite_nsym,ticket_type,price,price_usdt,sale_started_at,sale_ended_at);}


#define ISSUE_TO_GRAB(bank, to, quantity, memo) \
    {	flon::show::issuetograb_action act{ bank, { {get_self(), "active"_n} } };\
			act.send(  to, quantity, memo);}



class [[eosio::contract("show.cisum")]] show : public contract {
public:
  using contract::contract;

  // === cvticket.nft: 创建票种 ===
  [[eosio::action]]
  void nftcreate(const int64_t&  max_supply,
                const nsymbol&  symbol,
                const string&   token_uri);

  // === cvticket.nft: 发放（铸造到合约自身，再转出/或直接发放） ===
  [[eosio::action]]
  void nftissue(    const name&   issuer,
                    const nasset& quantity,
                    const string& memo);

  // ===== 演出 =====
  [[eosio::action]] void newshow(const uint64_t&   show_id,
                                  const name&       category,
                                  const bool&       ticket_transferable,
                                  const bool&       ticket_refundable,
                                  const time_point& show_started_at,
                                  const time_point& show_ended_at,
                                  const string&       show_name,
                                  const string&       show_address);

  [[eosio::action]] void setshow(const uint64_t&   show_id,
                                  const name&       category,
                                  const bool&       ticket_transferable,
                                  const bool&       ticket_refundable,
                                  const time_point& show_started_at,
                                  const time_point& show_ended_at,
                                  const string&       show_name,
                                  const string&       show_address);

  // ===== 票档 =====
  [[eosio::action]] void newticket(const uint64_t& show_id,
                                    const nsymbol&  ticket_nsym,
                                    const nsymbol&  prerequisite_nsym,
                                    const string&   ticket_type,
                                    const asset&    price,
                                    const asset&    price_usdt,
                                    const time_point& sale_started_at,
                                    const time_point& sale_ended_at);

  [[eosio::action]] void setticket(const uint64_t& show_id,
                                    const uint64_t& ticket_id,
                                    const string&   ticket_type,
                                    const asset&    price,
                                    const asset&    price_usdt,
                                    const time_point& sale_started_at,
                                    const time_point& sale_ended_at);

  // ===== 发放（从票档直接发 NFT）=====
  [[eosio::action]] void issue(const name&     user,
                               const uint64_t& show_id,
                               const uint64_t& ticket_id,
                               const uint32_t& amount,
                               const string&   memo);


  [[eosio::action]]
  void issuetograb(const name& to, const nasset& quantity, const string& memo);

  using newshow_action     = eosio::action_wrapper<"newshow"_n,&show::newshow>;
  using nftcreate_action     = eosio::action_wrapper<"nftcreate"_n,&show::nftcreate>;
  using nftissue_action     = eosio::action_wrapper<"nftissue"_n,&show::nftissue>;
  using newticket_action     = eosio::action_wrapper<"newticket"_n,&show::newticket>;
  using issuetograb_action     = eosio::action_wrapper<"issuetograb"_n,&show::issuetograb>;

};

} // namespace flon