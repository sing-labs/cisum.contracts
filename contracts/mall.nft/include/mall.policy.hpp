#pragma once
#include <eosio/eosio.hpp>
#include <flon/nft.transfer.hpp>
namespace mallnft {
using namespace eosio;
// Separate tables preserve the serialized layout of existing tokenstats rows.
struct [[eosio::table("issuance"), eosio::contract("mall.nft")]] issuance_row {
    uint64_t nft_id;
    int64_t minted = 0;
    int64_t reserved = 0;
    uint64_t primary_key() const { return nft_id; }
    EOSLIB_SERIALIZE(issuance_row, (nft_id)(minted)(reserved))
};
using issuance_table = multi_index<"issuance"_n, issuance_row>;
struct [[eosio::table("xferpolicy"), eosio::contract("mall.nft")]] xfer_policy {
    uint64_t nft_id;
    name manager;
    uint64_t primary_key() const { return nft_id; }
    EOSLIB_SERIALIZE(xfer_policy, (nft_id)(manager))
};
using policy_table = multi_index<"xferpolicy"_n, xfer_policy>;
// Scope is NFT id. No receiver/global-whitelist bypass for managed NFTs.
struct [[eosio::table("xferwhite"), eosio::contract("mall.nft")]] xfer_member {
    name account;
    uint64_t primary_key() const { return account.value; }
    EOSLIB_SERIALIZE(xfer_member, (account))
};
using xfer_table = multi_index<"xferwhite"_n, xfer_member>;
}
