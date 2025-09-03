#pragma once

#include <eosio/name.hpp>
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace eosio;
using namespace std;
struct CisumSummary {
    vector< extended_asset > tokens;

    CisumSummary( vector< extended_asset >& assets ): tokens(assets) {};

    EOSLIB_SERIALIZE(CisumSummary, (tokens))
};

//scope: account
struct [[eosio::table, eosio::contract("cisumsummary")]] accounts {
    eosio::asset balance;

    uint64_t primary_key() const { return balance.symbol.code().raw(); }
};

// 通用token资产表，不建议动，兼容多合约
typedef eosio::multi_index< "accounts"_n, accounts > tbl_accounts;