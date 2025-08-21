#pragma once

#include <eosio/asset.hpp>
#include <eosio/privileged.hpp>
#include <eosio/singleton.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>

#include <utils.hpp>

// #include <deque>
#include <optional>
#include <string>
#include <map>
#include <set>
#include <type_traits>



namespace flon {

using namespace std;
using namespace eosio;

#define HASH256(str) sha256(const_cast<char*>(str.c_str()), str.size())

#define TBL struct [[eosio::table, eosio::contract("pos.cisum")]]
#define NTBL(name) struct [[eosio::table(name), eosio::contract("pos.cisum")]]

namespace deposit_type {
    static constexpr eosio::name TERM       = "term"_n;
    static constexpr eosio::name DEMAND     = "demand"_n;
}

namespace pool_type {
    static constexpr eosio::name CISUM_APR = "cisumapr"_n;  // 池A：利息按时释放（CISUM）
    static constexpr eosio::name NEST_PONT = "nestpont"_n;  // 池B：一次性积分（NESTAR）
}
namespace interest_rate_scheme {
    static constexpr eosio::name LADDER3    = "lad3"_n;     //12-mo ladder ir
    static constexpr eosio::name LADDER2    = "lad2"_n;     //6-mo ladder ir
    static constexpr eosio::name LADDER1    = "lad1"_n;     //3-mo ladder ir

    static constexpr eosio::name LADDER31   = "lad31"_n;    //12-mo ladder ir
    static constexpr eosio::name LADDER21   = "lad21"_n;    //6-mo ladder ir
    static constexpr eosio::name LADDER11   = "lad11"_n;    //3-mo ladder ir

    static constexpr eosio::name LOG1       = "log1"_n;

    static constexpr eosio::name DEMAND1    = "dem1"_n;
    static constexpr eosio::name DEMAND2    = "dem2"_n;
    static constexpr eosio::name DEMAND3    = "dem3"_n;
}

NTBL("global") global_t {
    name admin                              = "cisumadmin"_n;
    name penalty_share_account              = "share.cisum"_n;
    extended_symbol     principal_token;            //E.g. 8,AMAX@amax.token, can be set differently for diff contract
    asset mini_deposit_amount;                      // 最小可存入本金
    uint64_t share_pool_id                  = 0;    //to be set a value which has been set for this contract as a whole
    uint64_t last_save_id                   = 0;

    EOSLIB_SERIALIZE( global_t, (admin)(penalty_share_account)(principal_token)(mini_deposit_amount)
                                (share_pool_id)(last_save_id) )

};
typedef eosio::singleton< "global"_n, global_t > global_singleton;

struct plan_conf_s {
    name           type;                     // 存款类型：deposit_type::TERM / deposit_type::DEMAND
    name           pool_type;                // 池型：pool_type::CISUM_APR | pool_type::NEST_PONT
    name           ir_scheme;                // 利率方案：interest_rate_scheme::*
    uint64_t       deposit_term_days;        // 期限天数 E.g. 365
    bool           allow_advance_redeem;     // 是否允许提前赎回
    uint64_t       advance_redeem_fine_rate; // 提前赎回罚金比例（万分制）
    time_point_sec effective_from;           // 生效期：早于此不允许存入
    time_point_sec effective_to;             // 截止期：晚于此不允许存入
    extended_symbol interest_token;          // 奖励币（CISUM/NESTAR 等），每计划独立

    EOSLIB_SERIALIZE( plan_conf_s,
        (type)(pool_type)(ir_scheme)(deposit_term_days)
        (allow_advance_redeem)(advance_redeem_fine_rate)
        (effective_from)(effective_to)(interest_token)
    )
};

//scope: self
TBL save_plan_t {
    uint64_t            id;                         //PK
    plan_conf_s         conf;                       // 方案配置
    asset               deposit_available;          //当前在本方案中已存本金累计
    asset               deposit_redeemed;           //已赎回本金累计
    asset               interest_available;         //案内可用于发放的利息池
    asset               interest_redeemed;          //已发放/提取的利息累计
    time_point_sec      created_at;

    save_plan_t() {}
    save_plan_t(const uint64_t& i): id(i) {}

    uint64_t primary_key()const { return id; }
    uint64_t scope()const { return 0; }

    typedef multi_index<"saveplans"_n, save_plan_t > tbl_t;

    EOSLIB_SERIALIZE( save_plan_t,  (id)(conf)
                                    (deposit_available)(deposit_redeemed)
                                    (interest_available)(interest_redeemed)
                                    (created_at) )

};

//Scope: account
//Note: record will be deleted upon withdrawal/redemption
TBL save_account_t {
    uint64_t            save_id;              //PK
    uint64_t            plan_id;
    uint64_t            interest_rate;        //boost by 10000
    asset               deposit_quant;        //存入本金
    asset               interest_term_quant;  //total interest collectable upon term completion
    asset               interest_collected;   //已领取利息累计
    time_point_sec      created_at;
    time_point_sec      term_ended_at;
    time_point_sec      last_collected_at;    //上次领取利息时间


    save_account_t() {}
    save_account_t(const uint64_t& i): save_id(i) {}

    uint64_t primary_key()const { return save_id; }
    uint64_t by_plan()const { return plan_id; }

    typedef multi_index<"saveaccounts"_n, save_account_t,
        indexed_by<"planid"_n, const_mem_fun<save_account_t, uint64_t, &save_account_t::by_plan> >
    > tbl_t;

    EOSLIB_SERIALIZE( save_account_t,   (save_id)(plan_id)(interest_rate)(deposit_quant)(interest_term_quant)(interest_collected)
                                        (created_at)(term_ended_at)(last_collected_at) )

};

} //namespace flon
