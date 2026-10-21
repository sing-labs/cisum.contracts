#pragma once

#include <eosio/asset.hpp>
#include <eosio/binary_extension.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/time.hpp>
#include <string>
#include <vector>

namespace flon {

using namespace eosio;

struct airdrop_item {
    eosio::name to;
    std::string quantity;

    EOSLIB_SERIALIZE(airdrop_item, (to)(quantity))
};

struct [[eosio::table, eosio::contract("cisumairdrop")]] airdrop_global {
    eosio::name admin;
    bool paused = false;

    EOSLIB_SERIALIZE(airdrop_global, (admin)(paused))
};

struct [[eosio::table, eosio::contract("cisumairdrop")]] airdrop_batch {
    uint64_t batch_id;
    eosio::name executor;
    eosio::asset total;
    uint32_t recipient_count;
    std::string memo;
    eosio::time_point_sec created_at;
    // Appended extension keeps existing CISUM batch rows readable after upgrade.
    eosio::binary_extension<eosio::name> token_contract;

    uint64_t primary_key() const { return batch_id; }
    EOSLIB_SERIALIZE(airdrop_batch, (batch_id)(executor)(total)(recipient_count)(memo)(created_at)(token_contract))
};

using airdrop_global_table = eosio::singleton<"global"_n, airdrop_global>;
using airdrop_batches = eosio::multi_index<"batches"_n, airdrop_batch>;

class [[eosio::contract("cisumairdrop")]] cisum_airdrop : public eosio::contract {
public:
    using contract::contract;

    [[eosio::action]] void init(const eosio::name& admin);
    [[eosio::action]] void setadmin(const eosio::name& admin);
    [[eosio::action]] void setpause(bool paused);
    [[eosio::action]] void batchdrop(uint64_t batch_id,
                                    const eosio::name& token_contract,
                                    const eosio::symbol& token_symbol,
                                    const std::vector<airdrop_item>& recipients,
                                    const std::string& memo);

    [[eosio::action, eosio::read_only]]
    airdrop_batch getbatch(uint64_t batch_id);

    [[eosio::action, eosio::read_only]]
    uint64_t lastbatch();

private:
    static eosio::asset parse_quantity(const std::string& quantity,
                                      const eosio::symbol& token_symbol);
    airdrop_global require_admin() const;
    eosio::asset get_token_balance(const eosio::name& token_contract,
                                  const eosio::symbol& token_symbol) const;
};

} // namespace flon
