#pragma once
#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <string>

using namespace eosio;
using std::string;

namespace flon {

class [[eosio::contract("flon.auth")]] flonauth : public contract {
public:
    using contract::contract;

    ACTION checkrole(const name& submitter,
                         const name& user,
                         const std::string& perm) ;

    using checkrole_action  = eosio::action_wrapper<"checkrole"_n,  &flonauth::checkrole>;

};

} // namespace flon