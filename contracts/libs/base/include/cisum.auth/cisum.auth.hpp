#pragma once
#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <string>

using namespace eosio;
using std::string;

namespace flon {

class [[eosio::contract("cisum.auth")]] cisumauth : public contract {
public:
    using contract::contract;

    ACTION checkrole(const name& submitter,
                         const name& user,
                         const std::string& perm) ;

    using checkrole_action  = eosio::action_wrapper<"checkrole"_n,  &cisumauth::checkrole>;

};

} // namespace flon