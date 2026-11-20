#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <eosio/crypto.hpp>
#include <eosio/binary_extension.hpp>
#include <flon/nasset.hpp>
#include <string>
#include <vector>
namespace mall {
using namespace eosio;
using std::string;
using std::vector;
using flon::nasset;
struct sale_terms {
    asset price;
    time_point_sec starts;
    uint32_t per_purchase;
    uint32_t per_account;
    EOSLIB_SERIALIZE(sale_terms, (price)(starts)(per_purchase)(per_account))
};
struct payment { name to; asset quantity; EOSLIB_SERIALIZE(payment, (to)(quantity)) };
struct role_accounts {
    vector<name> manager, treasurer, distributor, guardian;
    EOSLIB_SERIALIZE(role_accounts, (manager)(treasurer)(distributor)(guardian))
};
struct [[eosio::table("config"), eosio::contract("mall.cisum")]] config_row {
    name manager, treasurer, distributor, guardian;
    uint64_t next_nft = 6000000001ULL;
    bool buy_paused = false, drop_paused = false;
    binary_extension<role_accounts> roles; // Legacy storage only; never used for authorization.
    EOSLIB_SERIALIZE(config_row, (manager)(treasurer)(distributor)(guardian)(next_nft)(buy_paused)(drop_paused)(roles))
};
using config_table = singleton<"config"_n, config_row>;
// Separate singleton avoids reinterpreting legacy role names as a super-admin.
struct [[eosio::table("authconfig"), eosio::contract("mall.cisum")]] auth_config {
    name admin;
    EOSLIB_SERIALIZE(auth_config, (admin))
};
using auth_table = singleton<"authconfig"_n, auth_config>;
struct [[eosio::table("banks"), eosio::contract("mall.cisum")]] bank_row {
    name bank;
    bool nft;
    uint64_t primary_key() const { return bank.value; }
    EOSLIB_SERIALIZE(bank_row, (bank)(nft))
};
using banks = multi_index<"banks"_n, bank_row>;
// Scope = token contract; symbols include precision.
struct [[eosio::table("funds"), eosio::contract("mall.cisum")]] fund_row {
    symbol sym;
    int64_t sales = 0, drops = 0;
    uint64_t primary_key() const { return sym.raw(); }
    EOSLIB_SERIALIZE(fund_row, (sym)(sales)(drops))
};
using funds = multi_index<"funds"_n, fund_row>;
struct [[eosio::table("products"), eosio::contract("mall.cisum")]] product {
    uint64_t id;
    name nft_contract;
    uint64_t nft_id;
    bool mint;
    name pay_contract;
    sale_terms terms;
    int64_t quota = 0, sold = 0;
    uint8_t status = 0; // 0 offline, 1 on sale, 2 archived
    binary_extension<string> product_code; // Business ID; absent on pre-upgrade rows.
    uint64_t primary_key() const { return id; }
    checksum256 by_code() const {
        const auto code = product_code.has_value() ? product_code.value() : string{};
        return sha256(code.data(), code.size());
    }
    uint128_t by_asset() const { return (uint128_t(nft_contract.value) << 64) | nft_id; }
    EOSLIB_SERIALIZE(product, (id)(nft_contract)(nft_id)(mint)(pay_contract)(terms)(quota)(sold)(status)(product_code))
};
using products = multi_index<"products"_n, product,
    indexed_by<"byasset"_n, const_mem_fun<product, uint128_t, &product::by_asset>>,
    indexed_by<"bycode"_n, const_mem_fun<product, checksum256, &product::by_code>>>;
struct [[eosio::table("purchases"), eosio::contract("mall.cisum")]] purchase_row {
    uint64_t product_id;
    int64_t quantity = 0;
    uint64_t primary_key() const { return product_id; }
    binary_extension<string> product_code;
    EOSLIB_SERIALIZE(purchase_row, (product_id)(quantity)(product_code))
};
using purchases = multi_index<"purchases"_n, purchase_row>; // scope buyer
struct [[eosio::table("operations"), eosio::contract("mall.cisum")]] operation {
    uint64_t id;
    name actor, action;
    string reason;
    vector<char> before, after;
    time_point_sec at;
    uint64_t primary_key() const { return id; }
    EOSLIB_SERIALIZE(operation, (id)(actor)(action)(reason)(before)(after)(at))
};
using operations = multi_index<"operations"_n, operation>;
struct [[eosio::table("batches"), eosio::contract("mall.cisum")]] batch {
    uint64_t id;
    name bank;
    checksum256 digest;
    asset total;
    time_point_sec at;
    uint64_t primary_key() const { return id; }
    EOSLIB_SERIALIZE(batch, (id)(bank)(digest)(total)(at))
};
using batches = multi_index<"batches"_n, batch>;
class [[eosio::contract("mall.cisum")]] mall_contract : public contract {
public:
    using contract::contract;
    ACTION init(name admin);
    ACTION getproduct(string product_code);
    ACTION getpurchase(name buyer, string product_code);
    ACTION setcode(name submitter, uint64_t product_id, string product_code, uint64_t op_id, string reason);
    ACTION addbank(name submitter, name bank, bool nft, symbol sym);
    ACTION advancenft(name submitter, uint64_t next_id, uint64_t op_id, string reason);
    ACTION createnft(name submitter, name bank, string uri, name ipowner, int64_t maximum, int64_t quota,
                     name paybank, sale_terms terms, uint64_t op_id, string reason, string product_code);
    ACTION regstock(name submitter, name bank, uint64_t nft_id, int64_t quota, name paybank,
                    sale_terms terms, uint64_t op_id, string reason, string product_code);
    ACTION setsale(name submitter, uint64_t product_id, sale_terms terms, uint64_t op_id, string reason);
    ACTION setquota(name submitter, uint64_t product_id, int64_t quota, uint64_t op_id, string reason);
    ACTION setsupply(name submitter, uint64_t product_id, int64_t maximum, int64_t quota, uint64_t op_id, string reason);
    ACTION setmeta(name submitter, uint64_t product_id, string uri, uint64_t op_id, string reason);
    ACTION setstatus(name submitter, uint64_t product_id, uint8_t status, uint64_t op_id, string reason);
    ACTION withdrawnft(name submitter, uint64_t product_id, name to, int64_t quantity, uint64_t op_id, string reason);
    ACTION allocfund(name submitter, name bank, asset quantity, uint64_t op_id, string reason);
    ACTION airdrop(name submitter, uint64_t batch_id, name bank, vector<payment> entries);
    ACTION pause(name submitter, bool buying, bool dropping, uint64_t op_id, string reason);
    ACTION notifybuy(name buyer, string request_id, uint64_t product_id, name bank,
                     uint64_t nft_id, bool mint, int64_t quantity, asset paid);
    [[eosio::on_notify("*::transfer")]] void ontransfer();
private:
    config_row config() const;
    product find_code(const string& code) const;
    void unique_code(const string& code, uint64_t except_id = 0) const;
    name require_perm(name submitter, const string& perm) const;
    void terms_ok(name bank, const sale_terms& terms) const;
    void bank_ok(name bank, bool nft) const;
    int64_t balance(const product& p) const;
    void quota_ok(const product& p, int64_t quota) const;
    void sendnft(const product& p, name to, int64_t amount, const string& memo);
    void log(uint64_t id, name actor, name action, const string& reason,
             vector<char> before, vector<char> after);
    void receivepay(name from, name to, asset quantity, string memo);
    void receivenft(name from, name to, vector<nasset> assets, string memo);
};
}
