#include "/tmp/toolkit-account-build/flon.account/src/flon.account.cpp"


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("init:__eosio_action_init_flon_account"))) void __eosio_action_init_flon_account(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::name arg0; ds >> arg0;
flon::flon_account{eosio::name{r},eosio::name{c},ds}.init(arg0);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("setgasquant:__eosio_action_setgasquant_flon_account"))) void __eosio_action_setgasquant_flon_account(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::asset arg0; ds >> arg0;
flon::flon_account{eosio::name{r},eosio::name{c},ds}.setgasquant(arg0);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("newaccount:__eosio_action_newaccount_flon_account"))) void __eosio_action_newaccount_flon_account(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::name arg0; ds >> arg0;
eosio::public_key arg1; ds >> arg1;
eosio::name arg2; ds >> arg2;
flon::flon_account{eosio::name{r},eosio::name{c},ds}.newaccount(arg0, arg1, arg2);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("resetactive:__eosio_action_resetactive_flon_account"))) void __eosio_action_resetactive_flon_account(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::name arg0; ds >> arg0;
eosio::name arg1; ds >> arg1;
eosio::public_key arg2; ds >> arg2;
flon::flon_account{eosio::name{r},eosio::name{c},ds}.resetactive(arg0, arg1, arg2);
}}

extern "C" {
__attribute__((eosio_wasm_import))
void eosio_assert_code(uint32_t, uint64_t);	__attribute__((weak, eosio_wasm_entry, eosio_wasm_abi("{\"____comment\":\"This file was generated with eosio-abigen. DO NOT EDIT \",\"version\":\"flon::abi/1.2\",\"structs\":[{\"name\":\"account_record\",\"base\":\"\",\"fields\":[{\"name\":\"account\",\"type\":\"name\"}]},{\"name\":\"config\",\"base\":\"\",\"fields\":[{\"name\":\"admin\",\"type\":\"name\"},{\"name\":\"gas_quant\",\"type\":\"asset\"}]},{\"name\":\"init\",\"base\":\"\",\"fields\":[{\"name\":\"admin\",\"type\":\"name\"}]},{\"name\":\"newaccount\",\"base\":\"\",\"fields\":[{\"name\":\"admin\",\"type\":\"name\"},{\"name\":\"pubkey\",\"type\":\"public_key\"},{\"name\":\"acct\",\"type\":\"name\"}]},{\"name\":\"resetactive\",\"base\":\"\",\"fields\":[{\"name\":\"admin\",\"type\":\"name\"},{\"name\":\"acct\",\"type\":\"name\"},{\"name\":\"pubkey\",\"type\":\"public_key\"}]},{\"name\":\"setgasquant\",\"base\":\"\",\"fields\":[{\"name\":\"gas_quant\",\"type\":\"asset\"}]}],\"types\":[],\"actions\":[{\"name\":\"init\",\"type\":\"init\",\"ricardian_contract\":\"\"},{\"name\":\"newaccount\",\"type\":\"newaccount\",\"ricardian_contract\":\"\"},{\"name\":\"resetactive\",\"type\":\"resetactive\",\"ricardian_contract\":\"\"},{\"name\":\"setgasquant\",\"type\":\"setgasquant\",\"ricardian_contract\":\"\"}],\"tables\":[{\"name\":\"accounts\",\"type\":\"account_record\",\"index_type\":\"i64\",\"key_names\":[],\"key_types\":[]},{\"name\":\"config\",\"type\":\"config\",\"index_type\":\"i64\",\"key_names\":[],\"key_types\":[]}],\"ricardian_clauses\":[],\"variants\":[],\"abi_extensions\":[],\"action_results\":[]}")))
	void __insert_eosio_abi(unsigned long long r, unsigned long long c, unsigned long long a){eosio_assert_code(false, 1);}
}