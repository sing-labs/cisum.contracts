#include "/tmp/toolkit-flon.token/src/flon.token.cpp"


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("create:__eosio_action_create_token"))) void __eosio_action_create_token(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::name arg0; ds >> arg0;
eosio::asset arg1; ds >> arg1;
flon::token{eosio::name{r},eosio::name{c},ds}.create(arg0, arg1);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("issue:__eosio_action_issue_token"))) void __eosio_action_issue_token(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::name arg0; ds >> arg0;
eosio::asset arg1; ds >> arg1;
std::string arg2; ds >> arg2;
flon::token{eosio::name{r},eosio::name{c},ds}.issue(arg0, arg1, arg2);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("retire:__eosio_action_retire_token"))) void __eosio_action_retire_token(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::asset arg0; ds >> arg0;
std::string arg1; ds >> arg1;
flon::token{eosio::name{r},eosio::name{c},ds}.retire(arg0, arg1);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("transfer:__eosio_action_transfer_token"))) void __eosio_action_transfer_token(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::name arg0; ds >> arg0;
eosio::name arg1; ds >> arg1;
eosio::asset arg2; ds >> arg2;
std::string arg3; ds >> arg3;
flon::token{eosio::name{r},eosio::name{c},ds}.transfer(arg0, arg1, arg2, arg3);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("open:__eosio_action_open_token"))) void __eosio_action_open_token(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::name arg0; ds >> arg0;
eosio::symbol arg1; ds >> arg1;
eosio::name arg2; ds >> arg2;
flon::token{eosio::name{r},eosio::name{c},ds}.open(arg0, arg1, arg2);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("close:__eosio_action_close_token"))) void __eosio_action_close_token(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::name arg0; ds >> arg0;
eosio::symbol arg1; ds >> arg1;
flon::token{eosio::name{r},eosio::name{c},ds}.close(arg0, arg1);
}}

extern "C" {
__attribute__((eosio_wasm_import))
void eosio_assert_code(uint32_t, uint64_t);	__attribute__((weak, eosio_wasm_entry, eosio_wasm_abi("{\"____comment\":\"This file was generated with eosio-abigen. DO NOT EDIT \",\"version\":\"flon::abi/1.2\",\"structs\":[{\"name\":\"account\",\"base\":\"\",\"fields\":[{\"name\":\"balance\",\"type\":\"asset\"}]},{\"name\":\"close\",\"base\":\"\",\"fields\":[{\"name\":\"owner\",\"type\":\"name\"},{\"name\":\"symbol\",\"type\":\"symbol\"}]},{\"name\":\"create\",\"base\":\"\",\"fields\":[{\"name\":\"issuer\",\"type\":\"name\"},{\"name\":\"maximum_supply\",\"type\":\"asset\"}]},{\"name\":\"currency_stats\",\"base\":\"\",\"fields\":[{\"name\":\"supply\",\"type\":\"asset\"},{\"name\":\"max_supply\",\"type\":\"asset\"},{\"name\":\"issuer\",\"type\":\"name\"}]},{\"name\":\"issue\",\"base\":\"\",\"fields\":[{\"name\":\"to\",\"type\":\"name\"},{\"name\":\"quantity\",\"type\":\"asset\"},{\"name\":\"memo\",\"type\":\"string\"}]},{\"name\":\"open\",\"base\":\"\",\"fields\":[{\"name\":\"owner\",\"type\":\"name\"},{\"name\":\"symbol\",\"type\":\"symbol\"},{\"name\":\"ram_payer\",\"type\":\"name\"}]},{\"name\":\"retire\",\"base\":\"\",\"fields\":[{\"name\":\"quantity\",\"type\":\"asset\"},{\"name\":\"memo\",\"type\":\"string\"}]},{\"name\":\"transfer\",\"base\":\"\",\"fields\":[{\"name\":\"from\",\"type\":\"name\"},{\"name\":\"to\",\"type\":\"name\"},{\"name\":\"quantity\",\"type\":\"asset\"},{\"name\":\"memo\",\"type\":\"string\"}]}],\"types\":[],\"actions\":[{\"name\":\"close\",\"type\":\"close\",\"ricardian_contract\":\"\"},{\"name\":\"create\",\"type\":\"create\",\"ricardian_contract\":\"\"},{\"name\":\"issue\",\"type\":\"issue\",\"ricardian_contract\":\"\"},{\"name\":\"open\",\"type\":\"open\",\"ricardian_contract\":\"\"},{\"name\":\"retire\",\"type\":\"retire\",\"ricardian_contract\":\"\"},{\"name\":\"transfer\",\"type\":\"transfer\",\"ricardian_contract\":\"\"}],\"tables\":[{\"name\":\"accounts\",\"type\":\"account\",\"index_type\":\"i64\",\"key_names\":[],\"key_types\":[]},{\"name\":\"stat\",\"type\":\"currency_stats\",\"index_type\":\"i64\",\"key_names\":[],\"key_types\":[]}],\"ricardian_clauses\":[],\"variants\":[],\"abi_extensions\":[],\"action_results\":[]}")))
	void __insert_eosio_abi(unsigned long long r, unsigned long long c, unsigned long long a){eosio_assert_code(false, 1);}
}