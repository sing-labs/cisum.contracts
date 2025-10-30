#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>

#include <string>
#include <set>
using namespace eosio;
using std::string;

namespace flon {



struct [[eosio::table, eosio::contract("poe.cisum")]] rewardact_t {
  uint64_t   id;                        // 主键，自增
  name       reward_code;                  // 行为标识（signin / vote / short / invite / artist ...）
  asset      points;                    // 可领取积分（如 10.0000 SONG）
  string     description;               // 行为描述
  time_point create_at;
  time_point update_at;

  uint64_t primary_key() const { return id; }
  uint64_t by_name() const { return reward_code.value; }

  typedef eosio::multi_index<"rewardacts"_n, rewardact_t,
      indexed_by<"byname"_n, const_mem_fun<rewardact_t, uint64_t, &rewardact_t::by_name>>
      > acts_idx;
  EOSLIB_SERIALIZE(rewardact_t, (id)(reward_code)(points)(description)(create_at)(update_at))
};



} // namespace flon