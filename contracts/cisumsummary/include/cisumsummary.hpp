#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <string>
#include "cisumsummary.db.hpp"

namespace flon {

#define CONTRACT_VERSION "v1.0.0"

using std::string;
using namespace eosio;
class [[eosio::contract("cisumsummary")]] cisum_summary : public contract {
public:
    using contract::contract;

    ACTION addtoken(const name& bank, const symbol& sym);
    ACTION deltoken(const name& bank, const symbol& sym);

    [[eosio::action, eosio::read_only]]
    CisumSummary view(const name& account);

private:
    asset get_balance(const name& bank, const symbol& symb, const name& account) ;

    std::string format_amount(int64_t amount, uint8_t precision) ;
};

}