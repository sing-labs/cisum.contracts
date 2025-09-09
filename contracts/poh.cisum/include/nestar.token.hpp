#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <string>


using std::string;
using namespace eosio;
namespace flon {

static constexpr eosio::name active_perm{"active"_n};

#define NFT_ISSUE(bank, to, quantity, memo) \
    {	nestar::issue_action act{ bank, { {_self, active_perm} } };\
			act.send( to, quantity , memo );}

#define TRANSFER(bank,from ,to, quantity, memo) \
    {	nestar::transfer_action act{ bank, { {_self, active_perm} } };\
			act.send(from ,to, quantity , memo );}


class [[eosio::contract("nestar.token")]] nestar : public contract {
public:
  using contract::contract;


  [[eosio::action]]
  void issue(const name& to, const asset& quantity, const std::string& memo);

  // 允许：
  // 1) 发行者 -> 任意账户/合约
  // 2) 普通用户 -> 功能白名单账户/合约（如艺人账户、业务合约等）
  // 同时会在“用户支出场景”里累计 consumed 并触发自动勋章发放
  [[eosio::action]]
  void transfer(const name& from, const name& to, const asset& quantity, const std::string& memo);

  [[eosio::action]]
  void open(const name& owner, const symbol& sym, const name& ram_payer);


  using issue_action     = eosio::action_wrapper<"issue"_n,     &nestar::issue>;

  using transfer_action     = eosio::action_wrapper<"transfer"_n,     &nestar::transfer>;

};

} // namespace flon