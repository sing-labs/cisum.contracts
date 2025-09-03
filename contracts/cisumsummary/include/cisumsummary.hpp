#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <string>
#include "cisumsummary.db.hpp"

namespace flon {

#define CONTRACT_VERSION "v1.0.0"

using std::string;
using namespace eosio;

static constexpr name CISUM_BANK   = "cisum.token"_n;
static constexpr symbol CISUM      = symbol(symbol_code("CISUM"), 8);

static constexpr symbol MUSIC      = symbol(symbol_code("MUSIC"), 8);

static constexpr name NESTAR_BANK   = "nest21.token"_n;
static constexpr symbol NESTAR      = symbol(symbol_code("NESTAR"), 4);


class [[eosio::contract("cisumsummary")]] cisum_summary : public contract {
public:
    using contract::contract;

    [[eosio::action, eosio::read_only]]
    CisumSummary view(const name& account);

private:
    asset get_balance(const name& bank, const symbol& symb, const name& account) ;

    // std::string format_amount(int64_t amount, uint8_t precision) ;
};

}