#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <flon/consts.hpp>
#include <flon/flon.token.hpp>
#include <string>
#include <set>
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


struct [[eosio::table, eosio::contract("poe.cisum")]] global_t {
  asset      available_points = asset(0, CISUM_SYM); // 可用积分
  asset      claimed_points   = asset(0, CISUM_SYM); // 已发放累计
  asset      total_points     = asset(0, CISUM_SYM); // 总额度（= available + claimed）
  std::set<name>  operators;
  uint64_t   last_act_id      = 0;                      // 行为自增ID

  EOSLIB_SERIALIZE(global_t, (available_points)(claimed_points)(total_points)(operators)(last_act_id))
};
using global_singleton = eosio::singleton<"global"_n, global_t>;


struct [[eosio::table, eosio::contract("poe.cisum")]] rewardact_t {
  uint64_t   id;                        // 主键，自增
  name       act_name;                  // 行为标识（signin / vote / short / invite / artist ...）
  asset      points;                    // 可领取积分（如 10.0000 SONG）
  string     memo;                      // 备注
  asset      claimed_points = asset(0, CISUM_SYM); // 此行为已发放累计
  time_point create_at;
  time_point update_at;

  uint64_t primary_key() const { return id; }
  uint64_t by_name() const { return act_name.value; }

  typedef eosio::multi_index<"rewardacts"_n, rewardact_t,
      indexed_by<"byname"_n, const_mem_fun<rewardact_t, uint64_t, &rewardact_t::by_name>>
      > acts_idx;
  EOSLIB_SERIALIZE(rewardact_t, (id)(act_name)(points)(memo)(claimed_points)(create_at)(update_at))
};


struct  claim_info {
    name claimer;
    uint32_t cnt; //发放次数
};


} // namespace flon