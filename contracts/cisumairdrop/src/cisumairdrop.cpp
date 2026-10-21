#include "cisumairdrop.hpp"

#include <flon/consts.hpp>
#include <flon/flon.token.hpp>
#include <set>

namespace flon {

using namespace eosio;

airdrop_global cisum_airdrop::require_admin() const {
    airdrop_global_table globals(get_self(), get_self().value);
    check(globals.exists(), "contract not initialized");
    const auto state = globals.get();
    check(has_auth(get_self()) || has_auth(state.admin), "admin/contract only");
    return state;
}

void cisum_airdrop::init(const name& admin) {
    require_auth(get_self());
    airdrop_global_table globals(get_self(), get_self().value);
    check(!globals.exists(), "contract already initialized");
    check(is_account(admin), "admin account does not exist");
    globals.set(airdrop_global{admin, false}, get_self());
}

void cisum_airdrop::setadmin(const name& admin) {
    auto state = require_admin();
    check(is_account(admin), "admin account does not exist");
    state.admin = admin;
    airdrop_global_table(get_self(), get_self().value).set(state, get_self());
}

void cisum_airdrop::setpause(bool paused) {
    auto state = require_admin();
    state.paused = paused;
    airdrop_global_table(get_self(), get_self().value).set(state, get_self());
}

airdrop_batch cisum_airdrop::getbatch(uint64_t batch_id) {
    check(batch_id > 0, "batch_id must be positive");
    airdrop_batches batches(get_self(), get_self().value);
    auto batch = batches.get(batch_id, "batch not found");
    if (!batch.token_contract.has_value()) batch.token_contract = CISUM_CONTRACT;
    return batch;
}

uint64_t cisum_airdrop::lastbatch() {
    airdrop_batches batches(get_self(), get_self().value);
    if (batches.begin() == batches.end()) return 0;
    auto it = batches.end();
    --it;
    return it->batch_id;
}

asset cisum_airdrop::get_token_balance(const name& token_contract, const symbol& token_symbol) const {
    check(is_account(token_contract), "token contract account does not exist");
    check(token_symbol.is_valid(), "invalid token symbol");
    const auto supply = token::get_supply(token_contract, token_symbol.code());
    check(supply.symbol == token_symbol, "token symbol or precision mismatch");
    return token::get_balance(token_contract, get_self(), token_symbol.code());
}

asset cisum_airdrop::parse_quantity(const std::string& quantity, const symbol& token_symbol) {
    check(!quantity.empty() && quantity.size() <= 40, "invalid quantity format");
    int64_t amount = 0;
    bool decimal = false;
    uint32_t fractional_digits = 0;
    for (size_t i = 0; i < quantity.size(); ++i) {
        const char digit = quantity[i];
        if (digit == '.') {
            check(!decimal && i > 0 && i + 1 < quantity.size(), "invalid quantity format");
            decimal = true;
            continue;
        }
        check(digit >= '0' && digit <= '9', "invalid quantity format");
        if (decimal) {
            ++fractional_digits;
            check(fractional_digits <= token_symbol.precision(), "quantity exceeds token precision");
        }
        check(amount <= (asset::max_amount - (digit - '0')) / 10, "quantity overflow");
        amount = amount * 10 + (digit - '0');
    }
    // Pad missing decimal places without floating-point arithmetic or rounding.
    for (uint32_t i = fractional_digits; i < token_symbol.precision(); ++i) {
        check(amount <= asset::max_amount / 10, "quantity overflow");
        amount *= 10;
    }
    check(amount > 0, "quantity must be positive");
    return asset(amount, token_symbol);
}

void cisum_airdrop::batchdrop(uint64_t batch_id,
                             const name& token_contract,
                             const symbol& token_symbol,
                             const std::vector<airdrop_item>& recipients,
                             const std::string& memo) {
    const auto state = require_admin();
    check(!state.paused, "contract paused");
    check(batch_id > 0, "batch_id must be positive");
    check(!recipients.empty() && recipients.size() <= 200, "batch size must be between 1 and 200");
    check(memo.size() <= 256, "memo too long");
    check(token_symbol.is_valid(), "invalid token symbol");

    airdrop_batches batches(get_self(), get_self().value);
    check(batches.find(batch_id) == batches.end(), "batch already processed");

    asset total(0, token_symbol);
    std::set<name> accounts;
    std::vector<asset> quantities;
    quantities.reserve(recipients.size());
    for (const auto& item : recipients) {
        check(is_account(item.to), "recipient account does not exist");
        check(item.to != get_self(), "cannot airdrop to self");
        check(accounts.insert(item.to).second, "duplicate recipient in batch");
        const auto quantity = parse_quantity(item.quantity, token_symbol);
        check(quantity.amount <= asset::max_amount - total.amount, "batch total overflow");
        total += quantity;
        quantities.push_back(quantity);
    }

    // Read the authoritative token balance instead of maintaining a second ledger.
    const auto balance = get_token_balance(token_contract, token_symbol);
    check(balance >= total, "insufficient token balance");

    // Persist before sending inline actions so callbacks cannot replay this batch.
    // Any failed transfer rolls back this record and every transfer in the batch.
    batches.emplace(get_self(), [&](auto& row) {
        row.batch_id = batch_id;
        row.executor = has_auth(state.admin) ? state.admin : get_self();
        row.total = total;
        row.recipient_count = static_cast<uint32_t>(recipients.size());
        row.memo = memo;
        row.created_at = time_point_sec(current_time_point());
        row.token_contract = token_contract;
    });

    for (size_t i = 0; i < recipients.size(); ++i) {
        token::transfer_action{token_contract, {{get_self(), "active"_n}}}
            .send(get_self(), recipients[i].to, quantities[i], memo);
    }
}

} // namespace flon
