#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/permission.hpp>
#include <vector>
#include <string>
#include <set>
#include <flon/nasset.hpp>


static constexpr eosio::name active_perm{"active"_n};
#define NTOKEN_TRANSFER(bank,from_account ,to, quantity, memo) \
    {	flon::ntoken::transfer_action act{ bank, { {from_account, active_perm} } };\
			act.send( from_account, to, quantity , memo );}

namespace flon {

using std::string;
using std::vector;

using namespace eosio;

class [[eosio::contract("flon.ntoken")]] ntoken : public contract {
   public:
      using contract::contract;

   ACTION create( const name& issuer,
                  const int64_t& maximum_supply,
                  const nsymbol& symbol,
                  const string& token_uri,
                  const name& ipowner );
   ACTION issue( const name& to, const nasset& quantity, const string& memo );
   ACTION settokenuri(const uint64_t& symbid, const string& url);
   ACTION transfer( const name& from, const name& to, const vector<nasset>& assets, const string& memo );
   using create_action = action_wrapper< "create"_n, &ntoken::create >;
   using issue_action = action_wrapper< "issue"_n, &ntoken::issue >;
   using settokenuri_action = action_wrapper< "settokenuri"_n, &ntoken::settokenuri >;
   using transfer_action = action_wrapper< "transfer"_n, &ntoken::transfer >;

};
} //namespace flon
