#pragma once
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>

namespace flon {
using namespace eosio;
// Capability marker for the global sender policy. Missing means the legacy per-NFT protocol.
struct [[eosio::table("xfermode"), eosio::contract("mall.nft")]] nft_transfer_mode {
    uint8_t mode = 1;
    EOSLIB_SERIALIZE(nft_transfer_mode, (mode))
};
using nft_transfer_modes = eosio::singleton<"xfermode"_n, nft_transfer_mode>;

// Contract scope, deliberately distinct from legacy per-NFT xferwhite scopes.
struct [[eosio::table("senders"), eosio::contract("mall.nft")]] nft_sender {
    eosio::name account;
    uint64_t primary_key() const { return account.value; }
    EOSLIB_SERIALIZE(nft_sender, (account))
};
using nft_senders = eosio::multi_index<"senders"_n, nft_sender>;
}
