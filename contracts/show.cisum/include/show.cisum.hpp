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

#define CHECKC(exp, code, msg) \
   { if (!(exp)) eosio::check(false, string("[[") + std::to_string((int)code) + string("]] ") + msg); }

enum class err: uint8_t {
   INVALID_FORMAT         = 0,
   TYPE_INVALID           = 1,
   FEE_NOT_FOUND          = 2,
   INSUFFICIENT_QUANTITY  = 3,
   NOT_POSITIVE           = 4,
   SYMBOL_MISMATCH        = 5,
   EXPIRED                = 6,
   PWHASH_INVALID         = 7,
   RECORD_NO_FOUND        = 8,
   NOT_REPEAT_RECEIVE     = 9,
   NOT_EXPIRED            = 10,
   ACCOUNT_INVALID        = 11,
   FEE_NOT_POSITIVE       = 12,
   VAILD_TIME_INVALID     = 13,
   MIN_UNIT_INVALID       = 14,
   REDPACK_EXIST          = 15,
   DID_NOT_AUTH           = 16,
   UNDER_MAINTENANCE      = 17,
   NONE_DELETED           = 19,
   IN_THE_WHITELIST       = 20,
   NON_RENEWAL            = 21,
   AMOUNT_TOO_SMALL       = 22,
   AMOUNT_TOO_LARGE       = 23,
   FEE_NOT_REQUIRED       = 24,
   DID_NOT_SUPPORTED      = 25,
   DID_PACK_SYMBOL_ERR    = 26,
   STATUS_MISMATCH        = 27,
   EXCEED_LIMIT           = 28,
   QUANTITY_MISMATCH      = 29,
   INVALID_TIME           = 30
};

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
  ACTION init(const name& admin,const name& nft_bank);

  ACTION createnft(const name& submitter,
                    const int64_t& max_supply,
                    const nsymbol& symbol,
                    const string&  token_uri);

  ACTION issuenft( const name& submitter,
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

  // ===== 票档 =====
  ACTION newticket(const name& submitter,
                    const uint64_t&         show_id,
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

  ACTION delshow(const name& submitter, const uint64_t& show_id);

  // ===== 发放（从票档直接发 NFT）=====
  ACTION issue(const name&     submitter,
                 const name&                user,
                 const uint64_t&            show_id,
                 const uint64_t&            ticket_id,
                 const uint32_t&            ticket_count,
                 const string&              memo);

  // ===== 批量赠送 =====
  ACTION batchreward(const name&         oper,
                     const uint64_t&        show_id,
                     const uint64_t&        ticket_id,
                     const uint32_t&        ticket_count,
                     const vector<name>&    recipients,
                     const string&          memo);

  ACTION notenftissue(  const uint64_t&        show_id,
                      const uint64_t&        ticket_id,
                      const uint64_t&        ticket_count,
                      const uint64_t&        prev_ticket_count,
                      const name&            issuer,
                      const string&          memo,
                      const uint64_t&        created_at);


  ACTION buyticket(const name&  submitter,
                     const name&          payer,
                     const asset&         pay_amount,
                     const uint64_t&      show_id,
                     const uint64_t&      ticket_id,
                     const uint32_t&      ticket_count,
                     const string&        memo);


  ACTION issuetograb(const name&  submitter,const name& to, const nasset& quantity, const string& memo);

  ACTION retire(const uint64_t&  show_id,const nasset& quantity);

  using createnft_action      = eosio::action_wrapper<"createnft"_n,&show::createnft>;
  using notenftissue_action     = eosio::action_wrapper<"notenftissue"_n,&show::notenftissue>;
  using issue_action          = eosio::action_wrapper<"issue"_n,&show::issue>;


private:
  void require_perm(const name& submitter,
                        const std::string& roles) const;


private:
  global_singleton _global;
  global_t         _gstate;
};

} // namespace flon