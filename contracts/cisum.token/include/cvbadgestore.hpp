#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <string>
#include <vector>

using namespace eosio;
using std::string;
using std::vector;


static constexpr eosio::name active_perm{"active"_n};
#define BADGE_AWARD(badgestore_contract ,to, packs, memo) \
    {	flon::cvbadgestore::award_action act{ badgestore_contract, std::vector<eosio::permission_level>{ { get_self(), "active"_n } } };\
			act.send( get_self(), to, packs , memo );}

namespace flon {

class [[eosio::contract("cvbadgestore")]] cvbadgestore : public contract {
public:
  using contract::contract;

  [[eosio::action]] void award(const name& submitter,const name& to, const vector<nasset>& packs, const string& memo);

  using award_action    = action_wrapper<"award"_n,    &cvbadgestore::award>;


};
} // namespace flon