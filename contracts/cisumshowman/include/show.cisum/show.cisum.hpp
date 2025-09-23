#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <string>

namespace flon {

using namespace eosio;
using std::string;
using flon::nsymbol;

#define CREATE_NFT(bank,submitter,max_supply, symbol, token_uri) \
    { flon::show::nftcreate_action act{ bank, { permission_level{ submitter, "active"_n } } };\
      act.send(submitter, max_supply, symbol, token_uri); }

#define ISSUE_NFT(bank,submitter,issuer,quantity,memo) \
    { flon::show::nftissue_action act{ bank, { permission_level{ submitter, "active"_n } } };\
      act.send(submitter,issuer, quantity, memo); }

#define NEW_SHOW(bank,submitter,show_id,category,ticket_transferable,ticket_refundable,show_started_at,show_ended_at,show_name,show_address) \
    { flon::show::newshow_action act{ bank, { permission_level{ submitter, "active"_n } } };\
      act.send(submitter, show_id, category, ticket_transferable, ticket_refundable, show_started_at, show_ended_at, show_name, show_address); }

#define NEW_TICKET(bank,submitter,show_id,ticket_nsym,prerequisite_nsym,ticket_type,price,price_usdt,sale_started_at,sale_ended_at) \
    { flon::show::newticket_action act{ bank, { permission_level{ submitter, "active"_n } } };\
      act.send(submitter, show_id, ticket_nsym, prerequisite_nsym, ticket_type, price, price_usdt, sale_started_at, sale_ended_at); }

#define ISSUE_TO_GRAB(bank, submitter,to, quantity, memo) \
    { flon::show::issuetograb_action act{ bank, { permission_level{ submitter, "active"_n } } };\
      act.send(submitter, to, quantity, memo); }



class [[eosio::contract("show.cisum")]] show : public contract {
public:
  using contract::contract;


  [[eosio::action]]
  void nftcreate(const name& submitter,
                    const int64_t& max_supply,
                    const nsymbol& symbol,
                    const string&  token_uri);

  // === cvticket.nft: 发放（铸造到合约自身，再转出/或直接发放） ===
  [[eosio::action]]
  void nftissue(const name& submitter,
                    const name&   issuer,
                    const nasset& quantity,
                    const string& memo);

  // ===== 演出 =====
  ACTION newshow(const name& submitter,
                    const uint64_t&         show_id,
                    const name&             category,
                    const bool&             ticket_transferable,
                    const bool&             ticket_refundable,
                    const time_point&       show_started_at,
                    const time_point&       show_ended_at,
                    const string&           show_name,
                    const string&           show_address);

  ACTION setshow(const name& submitter,
                    const uint64_t&         show_id,
                    const name&             category,
                    const bool&             ticket_transferable,
                    const bool&             ticket_refundable,
                    const time_point&       show_started_at,
                    const time_point&       show_ended_at,
                    const string&           show_name,
                    const string&           show_address);

  ACTION newticket(const name& submitter,
                    const uint64_t&         how_id,
                    const nsymbol&          ticket_nsym,
                    const nsymbol&          prerequisite_nsym,
                    const string&           ticket_type,
                    const asset&            price,
                    const asset&            price_usdt,
                    const time_point&       sale_started_at,
                    const time_point&       sale_ended_at);

  ACTION setticket(const name& submitter,
                    const uint64_t&         show_id,
                    const uint64_t&         ticket_id,
                    const string&           ticket_type,
                    const asset&            price,
                    const asset&            price_usdt,
                    const time_point&       sale_started_at,
                    const time_point&       sale_ended_at);

  ACTION issue(const name&     submitter,
                 const name&                user,
                 const uint64_t&            show_id,
                 const uint64_t&            ticket_id,
                 const uint32_t&            ticket_count,
                 const string&              memo);


  ACTION issuetograb(const name&  submitter,const name& to, const nasset& quantity, const string& memo);

  using newshow_action     = eosio::action_wrapper<"newshow"_n,&show::newshow>;
  using nftcreate_action     = eosio::action_wrapper<"nftcreate"_n,&show::nftcreate>;
  using nftissue_action     = eosio::action_wrapper<"nftissue"_n,&show::nftissue>;
  using newticket_action     = eosio::action_wrapper<"newticket"_n,&show::newticket>;
  using issuetograb_action     = eosio::action_wrapper<"issuetograb"_n,&show::issuetograb>;

};

} // namespace flon