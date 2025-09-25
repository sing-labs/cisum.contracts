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

    ACTION addrewardact(const name& act_name,const asset& points,const string& memo);

    ACTION delrewardact(const name& act_name);

    ACTION addoracle(const name& account);

    ACTION deloracle(const name& account);

    ACTION claimpoints(const name& submitter, const name& claimer, const name& act_name);

    ACTION consumeact(const name& caller, const name& act_name, const asset& amount);

    [[eosio::on_notify("song.token::transfer")]]
    void ontransfer(const name& from, const name& to,const asset& quantity, const string& memo);


    // 便捷别名
    using addrewardact_action     = eosio::action_wrapper<"addrewardact"_n,     &poe_cisum::addrewardact>;
    using delrewardact_action     = eosio::action_wrapper<"delrewardact"_n,     &poe_cisum::delrewardact>;
    using claimpts_action         = eosio::action_wrapper<"claimpoints"_n,      &poe_cisum::claimpoints>;
    using consumeact_action         = eosio::action_wrapper<"consumeact"_n,      &poe_cisum::consumeact>;

};

} // namespace flon