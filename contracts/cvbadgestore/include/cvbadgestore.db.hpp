#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <string>
#include <vector>
#include <flon.ntoken.hpp>


using namespace eosio;
using std::string;
using std::vector;

namespace flon {

// ---------- 通用断言 ----------
#define CHECKC(exp, code, msg) \
  { if (!(exp)) eosio::check(false, string("[[") + std::to_string((int)(code)) + string("]] ") + (msg)); }

enum class err: uint8_t {
  INVALID_FORMAT      = 0,
  RECORD_NO_FOUND     = 8,
  ACCOUNT_INVALID     = 11,
  NOT_POSITIVE        = 4,
  DID_NOT_AUTH        = 16,
  REDPACK_EXIST       = 15,
  STATUS_MISMATCH     = 27
};


#define TBL  struct [[eosio::table, eosio::contract("cvbadgestore")]]
#define NTBL(name) struct [[eosio::table(name), eosio::contract("cvbadgestore")]]

NTBL("global") global_t {
  name   admin;           // 管理员（可管理白名单/配置）
  name   badge_contract;  // 勋章 nToken 合约（如 cvbadge.nft）
  name   badge_from;      // 勋章来源账户（库存持有者）

  EOSLIB_SERIALIZE(global_t, (admin)(badge_contract)(badge_from))
};
using global_singleton = singleton<"global"_n, global_t>;

TBL whitelist_t {
  name       contract;     
  time_point added_at;

  uint64_t primary_key() const { return contract.value; }

  typedef eosio::multi_index<"whitelist"_n, whitelist_t> idx_t;

  EOSLIB_SERIALIZE(whitelist_t, (contract)(added_at))
};

} // namespace flon