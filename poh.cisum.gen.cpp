#include "/workspace/contracts/poh.cisum/src/poh.cisum.cpp"


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("init:__eosio_action_init_poh_cisum"))) void __eosio_action_init_poh_cisum(unsigned long long r, unsigned long long c) {
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
flon::poh_cisum{eosio::name{r},eosio::name{c},ds}.init(arg0, arg1, arg2);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("setmaxissued:__eosio_action_setmaxissued_poh_cisum"))) void __eosio_action_setmaxissued_poh_cisum(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::asset arg0; ds >> arg0;
flon::poh_cisum{eosio::name{r},eosio::name{c},ds}.setmaxissued(arg0);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("setplatform:__eosio_action_setplatform_poh_cisum"))) void __eosio_action_setplatform_poh_cisum(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::name arg0; ds >> arg0;
flon::poh_cisum{eosio::name{r},eosio::name{c},ds}.setplatform(arg0);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("setregistrar:__eosio_action_setregistrar_poh_cisum"))) void __eosio_action_setregistrar_poh_cisum(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::name arg0; ds >> arg0;
flon::poh_cisum{eosio::name{r},eosio::name{c},ds}.setregistrar(arg0);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("addoperator:__eosio_action_addoperator_poh_cisum"))) void __eosio_action_addoperator_poh_cisum(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::name arg0; ds >> arg0;
flon::poh_cisum{eosio::name{r},eosio::name{c},ds}.addoperator(arg0);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("deloperator:__eosio_action_deloperator_poh_cisum"))) void __eosio_action_deloperator_poh_cisum(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::name arg0; ds >> arg0;
flon::poh_cisum{eosio::name{r},eosio::name{c},ds}.deloperator(arg0);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("batchairdrop:__eosio_action_batchairdrop_poh_cisum"))) void __eosio_action_batchairdrop_poh_cisum(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::name arg0; ds >> arg0;
std::vector<flon::airdrop_s> arg1; ds >> arg1;
std::string arg2; ds >> arg2;
flon::poh_cisum{eosio::name{r},eosio::name{c},ds}.batchairdrop(arg0, arg1, arg2);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("registreward:__eosio_action_registreward_poh_cisum"))) void __eosio_action_registreward_poh_cisum(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::name arg0; ds >> arg0;
eosio::name arg1; ds >> arg1;
eosio::name arg2; ds >> arg2;
flon::poh_cisum{eosio::name{r},eosio::name{c},ds}.registreward(arg0, arg1, arg2);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_notify("*::transfer:__eosio_notify_on_transfer_poh_cisum"))) void __eosio_notify_on_transfer_poh_cisum(unsigned long long r, unsigned long long c) {
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
flon::poh_cisum{eosio::name{r},eosio::name{c},ds}.on_transfer(arg0, arg1, arg2, arg3);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("setfundinfo:__eosio_action_setfundinfo_poh_cisum"))) void __eosio_action_setfundinfo_poh_cisum(unsigned long long r, unsigned long long c) {
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
std::string arg3; ds >> arg3;
std::optional<uint32_t> arg4; ds >> arg4;
std::optional<uint32_t> arg5; ds >> arg5;
flon::poh_cisum{eosio::name{r},eosio::name{c},ds}.setfundinfo(arg0, arg1, arg2, arg3, arg4, arg5);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("redeemfund:__eosio_action_redeemfund_poh_cisum"))) void __eosio_action_redeemfund_poh_cisum(unsigned long long r, unsigned long long c) {
size_t as = ::action_data_size();
void* buff = nullptr;
if (as > 0) {
buff = as >= 512 ? malloc(as) : alloca(as);
::read_action_data(buff, as);
}
eosio::datastream<const char*> ds{(char*)buff, as};
eosio::name arg0; ds >> arg0;
eosio::name arg1; ds >> arg1;
flon::poh_cisum{eosio::name{r},eosio::name{c},ds}.redeemfund(arg0, arg1);
}}


#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
extern "C" {
__attribute__((eosio_wasm_import))
uint32_t action_data_size();
__attribute__((eosio_wasm_import))
uint32_t read_action_data(void*, uint32_t);
__attribute__((weak, eosio_wasm_action("notifyreward:__eosio_action_notifyreward_poh_cisum"))) void __eosio_action_notifyreward_poh_cisum(unsigned long long r, unsigned long long c) {
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
eosio::name arg4; ds >> arg4;
std::string arg5; ds >> arg5;
uint64_t arg6; ds >> arg6;
flon::poh_cisum{eosio::name{r},eosio::name{c},ds}.notifyreward(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
}}

extern "C" {
__attribute__((eosio_wasm_import))
void eosio_assert_code(uint32_t, uint64_t);	__attribute__((weak, eosio_wasm_entry, eosio_wasm_abi("{\"____comment\":\"This file was generated with eosio-abigen. DO NOT EDIT \",\"version\":\"flon::abi/1.2\",\"structs\":[{\"name\":\"addoperator\",\"base\":\"\",\"fields\":[{\"name\":\"account\",\"type\":\"name\"}]},{\"name\":\"airdrop_s\",\"base\":\"\",\"fields\":[{\"name\":\"beneficiary\",\"type\":\"name\"},{\"name\":\"quantity\",\"type\":\"asset\"}]},{\"name\":\"batchairdrop\",\"base\":\"\",\"fields\":[{\"name\":\"oper\",\"type\":\"name\"},{\"name\":\"drops\",\"type\":\"airdrop_s[]\"},{\"name\":\"memo\",\"type\":\"string\"}]},{\"name\":\"deloperator\",\"base\":\"\",\"fields\":[{\"name\":\"account\",\"type\":\"name\"}]},{\"name\":\"extended_symbol\",\"base\":\"\",\"fields\":[{\"name\":\"sym\",\"type\":\"symbol\"},{\"name\":\"contract\",\"type\":\"name\"}]},{\"name\":\"fund_balance_s\",\"base\":\"\",\"fields\":[{\"name\":\"reward_title\",\"type\":\"string\"},{\"name\":\"available_quant\",\"type\":\"asset\"},{\"name\":\"reward_per_invitee\",\"type\":\"asset\"},{\"name\":\"start_time\",\"type\":\"time_point_sec\"},{\"name\":\"end_time\",\"type\":\"time_point_sec\"}]},{\"name\":\"global_t\",\"base\":\"\",\"fields\":[{\"name\":\"platform_acct\",\"type\":\"name\"},{\"name\":\"registrar\",\"type\":\"name\"},{\"name\":\"usdt_per_user\",\"type\":\"asset\"},{\"name\":\"max_issued\",\"type\":\"asset\"},{\"name\":\"sing_issued\",\"type\":\"asset\"},{\"name\":\"cisum_issued\",\"type\":\"asset\"}]},{\"name\":\"init\",\"base\":\"\",\"fields\":[{\"name\":\"platform\",\"type\":\"name\"},{\"name\":\"registrar\",\"type\":\"name\"},{\"name\":\"max_issued\",\"type\":\"asset\"}]},{\"name\":\"inviter_fund_t\",\"base\":\"\",\"fields\":[{\"name\":\"inviter\",\"type\":\"name\"},{\"name\":\"balances\",\"type\":\"pair_extended_symbol_fund_balance_s[]\"}]},{\"name\":\"notifyreward\",\"base\":\"\",\"fields\":[{\"name\":\"from\",\"type\":\"name\"},{\"name\":\"to\",\"type\":\"name\"},{\"name\":\"award_amount\",\"type\":\"asset\"},{\"name\":\"memo\",\"type\":\"string\"},{\"name\":\"reward_type\",\"type\":\"name\"},{\"name\":\"reward_ref_id\",\"type\":\"string\"},{\"name\":\"created_at\",\"type\":\"uint64\"}]},{\"name\":\"operators_t\",\"base\":\"\",\"fields\":[{\"name\":\"accounts\",\"type\":\"name[]\"}]},{\"name\":\"pair_extended_symbol_fund_balance_s\",\"base\":\"\",\"fields\":[{\"name\":\"first\",\"type\":\"extended_symbol\"},{\"name\":\"second\",\"type\":\"fund_balance_s\"}]},{\"name\":\"redeemfund\",\"base\":\"\",\"fields\":[{\"name\":\"oper\",\"type\":\"name\"},{\"name\":\"inviter\",\"type\":\"name\"}]},{\"name\":\"registreward\",\"base\":\"\",\"fields\":[{\"name\":\"submitter\",\"type\":\"name\"},{\"name\":\"inviter\",\"type\":\"name\"},{\"name\":\"invitee\",\"type\":\"name\"}]},{\"name\":\"setfundinfo\",\"base\":\"\",\"fields\":[{\"name\":\"inviter\",\"type\":\"name\"},{\"name\":\"sym\",\"type\":\"symbol\"},{\"name\":\"contract\",\"type\":\"name\"},{\"name\":\"reward_title\",\"type\":\"string\"},{\"name\":\"start_ts\",\"type\":\"uint32?\"},{\"name\":\"end_ts\",\"type\":\"uint32?\"}]},{\"name\":\"setmaxissued\",\"base\":\"\",\"fields\":[{\"name\":\"max_issued\",\"type\":\"asset\"}]},{\"name\":\"setplatform\",\"base\":\"\",\"fields\":[{\"name\":\"platform\",\"type\":\"name\"}]},{\"name\":\"setregistrar\",\"base\":\"\",\"fields\":[{\"name\":\"registrar\",\"type\":\"name\"}]}],\"types\":[],\"actions\":[{\"name\":\"addoperator\",\"type\":\"addoperator\",\"ricardian_contract\":\"\"},{\"name\":\"batchairdrop\",\"type\":\"batchairdrop\",\"ricardian_contract\":\"\"},{\"name\":\"deloperator\",\"type\":\"deloperator\",\"ricardian_contract\":\"\"},{\"name\":\"init\",\"type\":\"init\",\"ricardian_contract\":\"\"},{\"name\":\"notifyreward\",\"type\":\"notifyreward\",\"ricardian_contract\":\"\"},{\"name\":\"redeemfund\",\"type\":\"redeemfund\",\"ricardian_contract\":\"\"},{\"name\":\"registreward\",\"type\":\"registreward\",\"ricardian_contract\":\"\"},{\"name\":\"setfundinfo\",\"type\":\"setfundinfo\",\"ricardian_contract\":\"\"},{\"name\":\"setmaxissued\",\"type\":\"setmaxissued\",\"ricardian_contract\":\"\"},{\"name\":\"setplatform\",\"type\":\"setplatform\",\"ricardian_contract\":\"\"},{\"name\":\"setregistrar\",\"type\":\"setregistrar\",\"ricardian_contract\":\"\"}],\"tables\":[{\"name\":\"global\",\"type\":\"global_t\",\"index_type\":\"i64\",\"key_names\":[],\"key_types\":[]},{\"name\":\"inviterfund\",\"type\":\"inviter_fund_t\",\"index_type\":\"i64\",\"key_names\":[],\"key_types\":[]},{\"name\":\"operators\",\"type\":\"operators_t\",\"index_type\":\"i64\",\"key_names\":[],\"key_types\":[]}],\"ricardian_clauses\":[],\"variants\":[],\"abi_extensions\":[],\"action_results\":[]}")))
	void __insert_eosio_abi(unsigned long long r, unsigned long long c, unsigned long long a){eosio_assert_code(false, 1);}
}