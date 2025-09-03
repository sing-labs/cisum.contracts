#pragma once

#include <eosio/name.hpp>
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace eosio;
using namespace std;


#define TBL struct [[eosio::table, eosio::contract("cisumsummary")]]
#define NTBL(name) struct [[eosio::table(name), eosio::contract("cisumsummary")]]

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

//scope: account
struct [[eosio::table, eosio::contract("flon.token")]] accounts {
    eosio::asset balance;

    uint64_t primary_key() const { return balance.symbol.code().raw(); }
};
typedef eosio::multi_index< "accounts"_n, accounts > tbl_accounts;

// ---------------- 可配置的统计代币清单 ----------------
// 由合约维护的一张表，记录需要在 summary 中展示的 (bank, symbol) 组合
TBL tokencfg {
    name bank;   // 代币合约账户，例如：cisum.token / nest21.token
    symbol sym;  // 代币符号（含精度），例如：8,CISUM / 4,NESTAR

    // 复合主键： (bank.value << 64) | sym.code().raw()
    uint128_t primary_key() const {
        return ( (uint128_t)bank.value << 64 ) | (uint128_t)sym.code().raw();
    }

    EOSLIB_SERIALIZE(tokencfg, (bank)(sym))
};
typedef eosio::multi_index<"tokencfg"_n, tokencfg> token_cfg_t;


static inline uint128_t make_tokencfg_pk(const name& bank, const symbol& sym) {
    return ( (uint128_t)bank.value << 64 ) | (uint128_t)sym.code().raw();
}