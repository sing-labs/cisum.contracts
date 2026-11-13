#pragma once
#include <eosio/eosio.hpp>
namespace flon {
using namespace eosio;
// Read-only views of the NFT issuance protocol, independent of the backend source tree.
struct issuance_row {
    uint64_t nft_id;
    int64_t minted = 0;
    int64_t reserved = 0;
    uint64_t primary_key() const { return nft_id; }
    EOSLIB_SERIALIZE(issuance_row, (nft_id)(minted)(reserved))
};
using issuance_table = multi_index<"issuance"_n, issuance_row>;
}
