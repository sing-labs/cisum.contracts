#pragma once
#include <eosio/eosio.hpp>
#include "mall.nft.db.hpp"
#include "mall.policy.hpp"

namespace mallnft {
class [[eosio::contract("mall.nft")]] mall_nft : public contract {
public:
    using contract::contract;

    // Only the contract account can authorize creators. No implicit administrator bypass.
    ACTION setcreator(name creator, bool to_add);
    ACTION create(const name& issuer, const int64_t& maximum_supply, const nsymbol& symbol,
                  const string& token_uri, const name& ipowner);
    ACTION issue(const name& to, const nasset& quantity, const string& memo);
    ACTION retire(const nasset& quantity, const string& memo);
    ACTION transfer(const name& from, const name& to, const vector<nasset>& assets, const string& memo);
    ACTION settokenuri(const uint64_t& symbid, const string& url);

    // Issuer assigns a manager once. Each NFT has an independent sender whitelist.
    ACTION setpolicy(uint64_t nft_id, name manager);
    ACTION setxferwl(uint64_t nft_id, name account, bool allowed);
    ACTION setmaxsup(uint64_t nft_id, int64_t maximum);
    ACTION setreserve(uint64_t nft_id, int64_t reserved);
private:
    void sub_balance(const name& owner, const nasset& value);
    void add_balance(const name& owner, const nasset& value, const name& ram_payer);
};
}
