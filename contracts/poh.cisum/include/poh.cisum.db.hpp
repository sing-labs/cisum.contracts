#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <string>

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
   INVALID_TIME           = 30
};


#define CHECKC(exp, code, msg) \
  { if (!(exp)) eosio::check(false, string("[[") + std::to_string((int)code) + string("]] ") + msg); }

// -------- 常量 --------
static constexpr name   SWAP_CONTRACT   = "flon.swap"_n;      // 价格来源
static constexpr name   REWARD_BANK     = "cisum.token"_n;    // 奖励币合约
static constexpr symbol CISUM_SYM       = symbol(symbol_code("CISUM"), 8);
static constexpr symbol USDT_SYM        = symbol(symbol_code("USDT"), 6);

static const asset      MAX_REWARD      = asset(25'5000'0000'0000'0000, CISUM_SYM);

// -------- 表：全局 --------
struct [[eosio::table, eosio::contract("poh.cisum")]] global_t {
  name   platform_acct;                                   // 平台账户（奖励接收方）
  name   registrar;                                       // 合约调用账户
  asset  usd_per_user     = asset(20'000000, USDT_SYM);   // 默认 20 USDT
  asset  max_reward       = MAX_REWARD;                   // 全网最大发放
  asset  available_rewards= asset(0, CISUM_SYM);          // 可用奖励池（CISUM）
  asset  issued_rewards   = asset(0, CISUM_SYM);          // 已发放累计（CISUM）



  EOSLIB_SERIALIZE(global_t,
    (platform_acct)(registrar)(usd_per_user)
    (max_reward)(available_rewards)(issued_rewards))
};
using global_singleton = eosio::singleton<"global"_n, global_t>;

// -------- 表：防重（已领取） --------
struct [[eosio::table, eosio::contract("poh.cisum")]] claimed_t {
  name        user;
  time_point  claimed_at;

  uint64_t primary_key() const { return user.value; }
};
using claimed_idx = eosio::multi_index<"claimed"_n, claimed_t>;

} // namespace flon