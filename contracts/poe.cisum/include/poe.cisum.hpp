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


    ACTION addrewardact(const name& reward_code,const asset& points,const string& memo);

    ACTION delrewardact(const name& reward_code);

    ACTION addoperator(const name& account);

    ACTION deloperator(const name& account);

    ACTION batchclaim(const name& submitter,
                        const uint64_t& uid,
                        const std::vector<claim_s>& claims ) ;

    ACTION consumeact(const name& submitter, const name& reward_code, const asset& amount);

    [[eosio::on_notify("cisum.token::transfer")]]
    void ontransfer(const name& from, const name& to,const asset& quantity, const string& memo);
    // ACTION notifyreward(const name&  from,
    //                             const name&       to,
    //                             const asset&      award_amount,
    //                             const string&     memo,
    //                             const name&       reward_type,
    //                             const string&     reward_ref_id,
    //                             const uint64_t&   created_at);

    // 便捷别名
    using addrewardact_action     = eosio::action_wrapper<"addrewardact"_n,     &poe_cisum::addrewardact>;
    using delrewardact_action     = eosio::action_wrapper<"delrewardact"_n,     &poe_cisum::delrewardact>;
    using batchclaim_action       = eosio::action_wrapper<"batchclaim"_n,       &poe_cisum::batchclaim>;
    // using notifyreward_action      = eosio::action_wrapper<"notifyreward"_n,      &poe_cisum::notifyreward>;
private:
    global_singleton _global;
    global_t         _gstate;

    void _pay_points( const claim_s& claim_info );

    rewardact_t _get_act(const name& reward_code);
};

} // namespace flon