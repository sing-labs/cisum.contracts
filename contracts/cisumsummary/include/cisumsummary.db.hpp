#pragma once
#include <eosio/asset.hpp>
#include <eosio/name.hpp>

struct token_info {
    eosio::name code;     // 合约名
    std::string balance;  // 余额字符串
    std::string coin;
    EOSLIB_SERIALIZE(token_info, (code)(balance)(coin))
};

struct CisumSummary {
    std::vector<token_info> tokens;
    EOSLIB_SERIALIZE(CisumSummary, (tokens))
};


struct [[eosio::table, eosio::contract("cisumsummary")]] accounts {
    eosio::asset balance;
    uint64_t primary_key() const { return balance.symbol.code().raw(); }
};

// 通用token资产表，不建议动，兼容多合约
typedef eosio::multi_index< "accounts"_n, accounts > tbl_accounts;