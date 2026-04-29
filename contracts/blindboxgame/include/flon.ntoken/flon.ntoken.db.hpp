#pragma once

#include <eosio/asset.hpp>
#include <eosio/privileged.hpp>
#include <eosio/singleton.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>

// #include <deque>
#include <optional>
#include <string>
#include <map>
#include <set>
#include <type_traits>

namespace flon {

using namespace std;
using namespace eosio;

#define HASH256(str) sha256(const_cast<char*>(str.c_str()), str.size())
#define TBL struct [[eosio::table, eosio::contract("flon.ntoken")]]
#define NTBL(name) struct [[eosio::table(name), eosio::contract("flon.ntoken")]]

struct nsymbol {
    uint64_t nid = 0;

    nsymbol() = default;
    explicit nsymbol(const uint64_t& raw): nid(raw) {}
    bool operator<(const nsymbol& other) const {
        return nid < other.nid;
    }

    friend bool operator==(const nsymbol&, const nsymbol&);
    bool is_valid()const { return nid != 0; }
    uint64_t raw()const { return nid; }

    EOSLIB_SERIALIZE( nsymbol, (nid) )
};

bool operator==(const nsymbol& symb1, const nsymbol& symb2) {
    return symb1.nid == symb2.nid;
}


struct nasset {
    int64_t         amount;
    nsymbol         symbol;

    nasset() {}
    nasset(const uint64_t& nid): symbol(nid), amount(0) {}
    nasset(const int64_t& amt, const nsymbol& symb): amount(amt), symbol(symb) {}

    nasset& operator+=(const nasset& quantity) { 
        check( quantity.symbol.raw() == this->symbol.raw(), "nsymbol mismatch");
        this->amount += quantity.amount; return *this;
    } 
    nasset& operator-=(const nasset& quantity) { 
        check( quantity.symbol.raw() == this->symbol.raw(), "nsymbol mismatch");
        this->amount -= quantity.amount; return *this; 
    }

    bool is_valid()const { return symbol.is_valid(); }
    
    EOSLIB_SERIALIZE( nasset, (amount)(symbol) )
};

//Scope: self
TBL nstats_t {
    nasset          supply;
    nasset          max_supply;     // 1 means NFT-721 type
    string          token_uri;      // globally unique uri for token metadata { image, desc,..etc }
    name            ipowner;        // who owns the IP
    name            notary;         // who notarized the IP authenticity and owership
    name            issuer;         // who created/uploaded/issued this NFT
    time_point_sec  issued_at;
    time_point_sec  notarized_at;
    bool            paused;

    nstats_t() {};
    
    uint64_t primary_key()const     { return supply.symbol.nid; }
    uint64_t by_ipowner()const      { return ipowner.value; }
    uint64_t by_issuer()const       { return issuer.value; }
    uint128_t by_issuer_created()const { return (uint128_t) issuer.value << 64 | (uint128_t) issued_at.sec_since_epoch(); }
    checksum256 by_token_uri()const { return HASH256(token_uri); } // unique index

    typedef eosio::multi_index
    < "tokenstats"_n,  nstats_t,
        indexed_by<"ipowneridx"_n,      const_mem_fun<nstats_t, uint64_t, &nstats_t::by_ipowner> >,
        indexed_by<"issueridx"_n,       const_mem_fun<nstats_t, uint64_t, &nstats_t::by_issuer> >,
        indexed_by<"issuercreate"_n,    const_mem_fun<nstats_t, uint128_t, &nstats_t::by_issuer_created> >,
        indexed_by<"tokenuriidx"_n,     const_mem_fun<nstats_t, checksum256, &nstats_t::by_token_uri> >
    > idx_t;

    EOSLIB_SERIALIZE(nstats_t,  (supply)(max_supply)(token_uri)(ipowner)(notary)(issuer)(issued_at)(notarized_at)(paused) )
};

///Scope: owner's account
TBL account_t {
    nasset      balance;
    bool        paused = false;   //if true, it can no longer be transferred

    account_t() {}
    account_t(const nasset& asset): balance(asset) {}

    uint64_t primary_key()const { return balance.symbol.raw(); }

    EOSLIB_SERIALIZE(account_t, (balance)(paused) )

    typedef eosio::multi_index< "accounts"_n, account_t > idx_t;
};


///Scope: owner's account
TBL allowance_t{
    name                        spender;                     // PK
    map<uint64_t, uint64_t>     allowances;                 // KV : NFT nid -> amount

    allowance_t() {}
    uint64_t primary_key()const { return spender.value; }

    EOSLIB_SERIALIZE(allowance_t, (spender)(allowances) )

    typedef eosio::multi_index< "allowances"_n, allowance_t > idx_t;
};

TBL creator_whitelist_t{

    name creator;
    creator_whitelist_t() {}

    uint64_t primary_key()const { return creator.value; }
    
    EOSLIB_SERIALIZE(creator_whitelist_t, (creator))

    typedef eosio::multi_index< "whitecreator"_n, creator_whitelist_t > idx_t;
};


} //namespace flon
