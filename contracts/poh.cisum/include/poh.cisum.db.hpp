#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <string>
#include <flon/consts.hpp>

using namespace eosio;
using std::string;

namespace flon {


enum class err: uint8_t {
   INVALID_FORMAT         = 0,
   TYPE_INVALID           = 1,
   FEE_NOT_FOUND          = 2,
   INSUFFICIENT_QUANTITY  = 3,
   NOT_POSITIVE           = 4,
   SYMBOL_MISMATCH        = 5,
   EXPIRED                = 6,
   PWHASH_INVALID         = 7,
   RECORD_NO_FOUND        = 8,
   NOT_REPEAT_RECEIVE     = 9,
   NOT_EXPIRED            = 10,
   ACCOUNT_INVALID        = 11,
   FEE_NOT_POSITIVE       = 12,
   VAILD_TIME_INVALID     = 13,
   MIN_UNIT_INVALID       = 14,
   REDPACK_EXIST          = 15,
   DID_NOT_AUTH           = 16,
   UNDER_MAINTENANCE      = 17,
   NONE_DELETED           = 19,
   IN_THE_WHITELIST       = 20,
   NON_RENEWAL            = 21,
   AMOUNT_TOO_SMALL       = 22,
   AMOUNT_TOO_LARGE       = 23,
   FEE_NOT_REQUIRED       = 24,
   DID_NOT_SUPPORTED      = 25,
   DID_PACK_SYMBOL_ERR    = 26,
   STATUS_MISMATCH        = 27,
   EXCEED_LIMIT           = 28,
   QUANTITY_MISMATCH      = 29,
   INVALID_TIME           = 30,
   PARAM_ERROR            = 31,
   QUANTITY_INSUFFICIENT  = 32
};


#define CHECKC(exp, code, msg) \
  { if (!(exp)) eosio::check(false, string("[[") + std::to_string((int)code) + string("]] ") + msg); }


static const asset      MAX_REWARD      = asset(25'5000'0000'0000'0000, SING_SYM);

//发 100 CISUM 给注册者
static const asset      CISUM_BONUS    = asset(100'0000, CISUM_SYM);


// -------- 表：全局 --------
struct [[eosio::table, eosio::contract("poh.cisum")]] global_t {
  name   platform_acct;                                   // 平台账户（奖励接收方）
  name   registrar;                                       // 合约调用账户
  asset  usdt_per_user     = asset(20'000000, USDT_SYM);   // 默认 20 USDT
  asset  max_issued        = asset(0, SING_SYM);          // 最大发放（SING）
  asset  sing_issued       = asset(0, SING_SYM);          // 已发放累计（SING）
  asset  cisum_issued     = asset(0, CISUM_SYM);         // 已发放累计（CISUM）


  EOSLIB_SERIALIZE(global_t,
    (platform_acct)(registrar)(usdt_per_user)
    (max_issued)(sing_issued)(cisum_issued))
};
using global_singleton = eosio::singleton<"global"_n, global_t>;

struct fund_balance_s {
    asset             available_quant;          // 可用余额
    asset             reward_per_invitee;       // 邀请奖励额度（同 symbol）
    time_point_sec    start_time;
    time_point_sec    end_time;
};


struct [[eosio::table, eosio::contract("poh.cisum")]] inviter_fund_t {  //scope: _self
    name                                        inviter;                // PK，邀请人账号
    std::map<extended_symbol, fund_balance_s>   balances;               // 多币种奖励池

    uint64_t primary_key() const { return inviter.value; }

    inviter_fund_t() {}
    inviter_fund_t(const name& inviter): inviter(inviter) {}

    typedef eosio::multi_index<"inviterfunds"_n, inviter_fund_t> tbl_t;

    EOSLIB_SERIALIZE( inviter_fund_t, (inviter)(balances) )
};


} // namespace flon