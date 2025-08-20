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


   ACTION transfer( const name& from, const name& to, const vector<nasset>& assets, const string& memo );
   using transfer_action = action_wrapper< "transfer"_n, &ntoken::transfer >;

};
} //namespace flon
