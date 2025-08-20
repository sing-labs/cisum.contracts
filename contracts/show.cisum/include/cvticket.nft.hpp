#pragma once
#include <eosio/eosio.hpp>
#include <string>
#include <vector>
#include <flon/nasset.hpp>

namespace flon {
using namespace eosio;
class [[eosio::contract("cvticket.nft")]] cvticket : public contract {
public:
  using contract::contract;

  // 仅声明 transfer 动作及其 action_wrapper
  [[eosio::action]]
  void transfer(const name& from, const name& to, const std::vector<nasset>& assets, const std::string& memo);

  using transfer_action = action_wrapper<"transfer"_n, &cvticket::transfer>;
};

} // namespace flon