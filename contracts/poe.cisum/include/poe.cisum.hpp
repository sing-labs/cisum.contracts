#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <string>

#include "poe.cisum.db.hpp"

using namespace eosio;
using std::string;

namespace flon {

class [[eosio::contract("poe.cisum")]] poe_cisum : public contract {
public:
    using contract::contract;

    poe_cisum(eosio::name receiver, eosio::name code, datastream<const char*> ds)
    : contract(receiver, code, ds),
      _global(get_self(), get_self().value)
    {
      _gstate = _global.exists() ? _global.get() : global_t{};
    }

    ~poe_cisum() { _global.set(_gstate, get_self()); }


    ACTION addrewardact(const name& act_name,const asset& points,const string& memo);

    ACTION delrewardact(const name& act_name);

    ACTION addoracle(const name& account);

    ACTION deloracle(const name& account);

    ACTION claimpoints(const name& leader, const name& claimer, const uint64_t& rewardact_id);

    [[eosio::on_notify("nest21.token::transfer")]]
    void ontransfer(const name& from, const name& to,const asset& quantity, const string& memo);


    // 便捷别名
    using addrewardact_action     = eosio::action_wrapper<"addrewardact"_n,     &poe_cisum::addrewardact>;
    using delrewardact_action     = eosio::action_wrapper<"delrewardact"_n,     &poe_cisum::delrewardact>;
    using claimpts_action         = eosio::action_wrapper<"claimpoints"_n,      &poe_cisum::claimpoints>;

private:
    global_singleton _global;
    global_t         _gstate;

    void _pay_points(const name& to, const asset& quant, const string& memo);

    rewardact_t _get_act(const name& act_name);
};

} // namespace flon