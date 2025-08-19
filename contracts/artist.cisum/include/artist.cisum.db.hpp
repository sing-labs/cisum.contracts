#pragma once
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
#include <string>
#include <set>




namespace flon {

using namespace eosio;
using std::string;
using std::set;


#define TBL struct [[eosio::table, eosio::contract("artist.cisum")]]
#define NTBL(name) struct [[eosio::table(name), eosio::contract("artist.cisum")]]
// ===== 全局配置 =====
NTBL("global") global_t {
  name        admin;                 // 平台管理员
  set<name>   auditors;                  // 审核员
  EOSLIB_SERIALIZE(global_t, (admin)(auditors))
};
typedef eosio::singleton< "global"_n, global_t > global_singleton;

// ===== 艺人/创作者主数据 =====
TBL artist_t {
    name        account;               // 主键：艺人/创作者账户
    string      display_name;          // 展示名
    string      avatar_url;            // 头像
    string      banner_url;            // 顶图
    string      bio;                   // 简介（短文本）
    string      country;               // 国家/地区代码
    string      language;              // 主语言（ISO 639-1）
    string      links;                 // JSON/URL
    name        status ;               // 是否启用
    name        level;                 // 级别（如：普通用户、认证用户、VIP 等）    
    bool        verified  = false;     // 平台认证标记
    time_point  created_at;
    time_point  updated_at;

    // 索引
    uint64_t primary_key()      const { return account.value; }
    uint64_t by_status()        const { return status.value; }
    uint128_t by_created()      const { return (uint128_t)created_at.sec_since_epoch(); }

    typedef eosio::multi_index<
    "artists"_n, artist_t,
    indexed_by<"bystatus"_n,  const_mem_fun<artist_t,uint64_t,&artist_t::by_status>>,
    indexed_by<"bycreated"_n, const_mem_fun<artist_t,uint128_t,&artist_t::by_created>>
    > idx_t;

    EOSLIB_SERIALIZE(artist_t,
    (account)(display_name)(avatar_url)(banner_url)(bio)(country)(language)(links)
    (status)(level)(verified)(created_at)(updated_at))
};



}