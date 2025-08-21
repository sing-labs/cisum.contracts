#pragma once
#include <eosio/eosio.hpp>
#include <string>
#include <vector>
#include <flon/nasset.hpp>
using std::string;
using std::vector;

namespace flon {
using namespace eosio;
class [[eosio::contract("cvticket.nft")]] cvticket : public contract {
public:
  using contract::contract;

  ACTION create( const name& issuer, const int64_t& maximum_supply, const nsymbol& symbol, const string& token_uri, const name& ipowner );

  ACTION issue( const name& to, const nasset& quantity, const string& memo );

  ACTION transfer(const name& from, const name& to, const std::vector<nasset>& assets, const std::string& memo);



  using transfer_action = action_wrapper<"transfer"_n, &cvticket::transfer>;
  using create_action   = action_wrapper<"create"_n, &cvticket::create>;
  using issue_action    = action_wrapper<"issue"_n, &cvticket::issue>;
};

} // namespace flon