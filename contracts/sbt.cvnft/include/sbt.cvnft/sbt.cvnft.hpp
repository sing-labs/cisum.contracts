#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/permission.hpp>

#include <string>

#include <sbt.cvnft/sbt.cvnft.db.hpp>

namespace flon {

using std::string;
using std::vector;
using std::to_string;
using namespace eosio;

static constexpr uint8_t MAX_BALANCE_COUNT = 30;
static constexpr name DID_CONTRACT = "did.ntoken"_n;
static constexpr uint32_t DID_SYMBOL_ID = 1000001;

#define CHECKC(exp, code, msg) \
   { if (!(exp)) eosio::check(false, string("[[") + to_string((int)code) + string("]] ") + msg); }

enum class err: uint8_t {
   INVALID_FORMAT       = 0,
   TYPE_INVALID         = 1,
   FEE_NOT_FOUND        = 2,
   INSUFFICIENT_QUANTITY  = 3,
   NOT_POSITIVE         = 4,
   SYMBOL_MISMATCH      = 5,
   EXPIRED              = 6,
   PWHASH_INVALID       = 7,
   RECORD_NO_FOUND      = 8,
   NOT_REPEAT_RECEIVE   = 9,
   NOT_EXPIRED          = 10,
   ACCOUNT_INVALID      = 11,
   FEE_NOT_POSITIVE     = 12,
   VAILD_TIME_INVALID   = 13,
   MIN_UNIT_INVALID     = 14,
   REDPACK_EXIST       = 15,
   DID_NOT_AUTH         = 16,
   UNDER_MAINTENANCE    = 17,
   NONE_DELETED         = 19,
   IN_THE_WHITELIST     = 20,
   NON_RENEWAL          = 21,
   AMOUNT_TOO_SMALL     = 22,
   AMOUNT_TOO_LARGE     = 23,
   FEE_NOT_REQUIRED     = 24,
   DID_NOT_SUPPORTED    = 25,
   DID_PACK_SYMBOL_ERR  = 26,
   STATUS_MISMATCH       = 27
};



/**
 * The `sbt.cvnft` sample system contract defines the structures and actions that allow users to create, issue, and manage tokens for AMAX based blockchains. It demonstrates one way to implement a smart contract which allows for creation and management of tokens. It is possible for one to create a similar contract which suits different needs. However, it is recommended that if one only needs a token with the below listed actions, that one uses the `flon.ntoken` contract instead of developing their own.
 *
 * The `sbt.cvnft` contract class also implements two useful public static methods: `get_supply` and `get_balance`. The first allows one to check the total supply of a specified token, created by an account and the second allows one to check the balance of a token for a specified account (the token creator account has to be specified as well).
 *
 * The `sbt.cvnft` contract manages the set of tokens, accounts and their corresponding balances, by using two internal multi-index structures: the `accounts` and `stats`. The `accounts` multi-index table holds, for each row, instances of `account` object and the `account` object holds information about the balance of one token. The `accounts` table is scoped to an eosio account, and it keeps the rows indexed based on the token's symbol.  This means that when one queries the `accounts` multi-index table for an account name the result is all the tokens that account holds at the moment.
 *
 * Similarly, the `stats` multi-index table, holds instances of `currency_stats` objects for each row, which contains information about current supply, maximum supply, and the creator account for a symbol token. The `stats` table is scoped to the token symbol.  Therefore, when one queries the `stats` table for a token symbol the result is one single entry/row corresponding to the queried symbol token if it was previously created, or nothing, otherwise.
 */
class [[eosio::contract("sbt.cvnft")]] cvnft : public contract {
   public:
      using contract::contract;

      cvnft(eosio::name receiver, eosio::name code, datastream<const char*> ds): contract(receiver, code, ds),
         _global(get_self(), get_self().value)
      {
         _gstate = _global.exists() ? _global.get() : global_t{};
      }

      ~cvnft() { _global.set( _gstate, get_self() ); }

      // ===== 空投白名单 =====
      ACTION setairdrop(name from, bool allow);

      ACTION create( const name& issuer, const int64_t& maximum_supply, const nsymbol& symbol, const string& token_uri, const name& ipowner );

      ACTION notarize(const name& notary, const uint32_t& token_id);
      ACTION setcreator( const name& creator, const bool& to_add);
      ACTION setnotary(const name& notary, const bool& to_add);

      ACTION setipowner(const uint64_t& symbid, const name& ip_owner);

      ACTION settokenuri(const uint64_t& symbid, const string& url);

      //外部合约调用空投过来:创建灵魂币或者更新灵魂币
      ACTION issue( const name& to, const nasset& quantity, const string& memo );

      ACTION transfer(const name& from, const name& to, const nasset& quantity, const string& memo);


      ACTION notifyreward(const name& predator, const name& victim, const asset& reward_quantity);


      using transfer_action = eosio::action_wrapper<"transfer"_n, &cvnft::transfer>;
      using issue_action = eosio::action_wrapper<"issue"_n, &cvnft::issue>;
      using create_action = eosio::action_wrapper<"create"_n, &cvnft::create>;
      using setairdrop_action = eosio::action_wrapper<"setairdrop"_n, &cvnft::setairdrop>;
      using notarize_action = eosio::action_wrapper<"notarize"_n, &cvnft::notarize>;
      using setcreator_action = eosio::action_wrapper<"setcreator"_n, &cvnft::setcreator>;
      using setnotary_action = eosio::action_wrapper<"setnotary"_n, &cvnft::setnotary>;
      using setipowner_action = eosio::action_wrapper<"setipowner"_n, &cvnft::setipowner>;
      using settokenuri_action = eosio::action_wrapper<"settokenuri"_n, &cvnft::settokenuri>;   
                       

      static nasset get_balance(const name& contract, const name& owner, const nsymbol& sym) { 
         auto acnts = flon::account_t::idx_t( contract, owner.value ); 
         const auto& acnt = acnts.get( sym.raw(), "no balance object found" ); 
         return acnt.paused? nasset(0, sym) : acnt.balance; 
      } 

      static uint64_t get_balance_by_parent( const name& contract, const name& owner, const uint32_t& pid ) { 
         auto ntable = flon::nstats_t::idx_t( contract, owner.value ); 
         auto idx = ntable.get_index<"parentidx"_n>(); 
         uint64_t id_lowest = (uint64_t)pid * 1E10; 
         auto itr = ntable.lower_bound( id_lowest ); 
         uint64_t amount = 0; 
         for (uint8_t i = 0; itr != ntable.end() && itr->supply.symbol.pid == pid; itr++, i++) { 
            if(i == MAX_BALANCE_COUNT) break; 
            auto acnts = flon::account_t::idx_t( contract, owner.value ); 
            auto sym = itr->supply.symbol; 
            auto acnt = acnts.find( sym.raw() ); 
            if(acnt == acnts.cend()) amount += 0; 
            else amount += acnt->paused? 0:acnt->balance.amount; 
         } 
         return amount; 
      }


   private:
         
      void add_balance( const name& owner, const nasset& value, const name& ram_payer );
      void sub_balance( const name& owner, const nasset& value );
      void _creator_auth_check( const name& creator);

   private:
      global_singleton     _global;
      global_t             _gstate;

};
} //namespace flon
