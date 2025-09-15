#pragma once
#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <set>
#include <string>

using namespace eosio;
using std::string;

namespace flon {

// -------- 错误码 --------
enum class err: uint8_t {
   INVALID_FORMAT     = 0,
   RECORD_NOT_FOUND   = 1,
   ROLE_EXISTS        = 2,
   ROLE_NOT_FOUND     = 3,
   GRANT_EXISTS       = 4,
   GRANT_NOT_FOUND    = 5,
   ACCOUNT_INVALID    = 6,
   PERMISSION_DENIED  = 7
};

#define CHECKC(exp, code, msg) \
  { if (!(exp)) eosio::check(false, string("[[") + std::to_string((int)code) + string("]] ") + msg); }

// -------- 全局配置（管理员 / 执行白名单） --------
struct [[eosio::table, eosio::contract("rolemanage")]] global_t {
  name           admin;    // 超级管理员（可维护角色&授权）
  std::set<name> oracles;  // 受信执行人（允许授权/撤权等）
  EOSLIB_SERIALIZE(global_t, (admin)(oracles))
};
using global_singleton = eosio::singleton<"global"_n, global_t>;

// -------- 表：角色配置 --------
struct [[eosio::table("roles"), eosio::contract("rolemanage")]] role_t {
    uint64_t          id;         // 自增主键
    name              role;       // 角色名（如 show_admin / show_editor / show_verifier_123）
    std::string       desc;       // 描述
    time_point        created_at; // 创建时间

    uint64_t primary_key() const { return id; }
    uint64_t by_role()     const { return role.value; }

    EOSLIB_SERIALIZE(role_t, (id)(role)(desc)(created_at))
};
using roles_idx = multi_index<
    "roles"_n, role_t,
    indexed_by<"byrole"_n, const_mem_fun<role_t, uint64_t, &role_t::by_role>>
>;

// -------- 表：用户角色绑定 --------
struct [[eosio::table("userroles"), eosio::contract("rolemanage")]] user_role_t {
    uint64_t          id;         // 自增主键
    name              contract;   // 业务合约名（如 show.cisum / grab.cisum）
    name              user;       // 被授予用户
    name              role;       // 角色名
    name              granter;    // 授权人
    time_point        created_at; // 授权时间

    uint64_t  primary_key()  const { return id; }

    uint64_t  by_user()      const { return user.value; }
    uint128_t by_user_role() const { return ((uint128_t)contract.value << 64) | role.value; }
    uint128_t by_user_contr()const { return ((uint128_t)user.value << 64) | contract.value; }
    uint64_t  by_role()      const { return role.value; }
    uint64_t  by_contr()     const { return contract.value; }

    EOSLIB_SERIALIZE(user_role_t, (id)(contract)(user)(role)(granter)(created_at))
};

using userroles_idx = multi_index<
    "userroles"_n, user_role_t,
    indexed_by<"byuser"_n,     const_mem_fun<user_role_t, uint64_t,  &user_role_t::by_user>>,
    indexed_by<"byuserrole"_n, const_mem_fun<user_role_t, uint128_t, &user_role_t::by_user_role>>,
    indexed_by<"byusercontr"_n,const_mem_fun<user_role_t, uint128_t, &user_role_t::by_user_contr>>,
    indexed_by<"byrole"_n,     const_mem_fun<user_role_t, uint64_t,  &user_role_t::by_role>>,
    indexed_by<"bycontr"_n,    const_mem_fun<user_role_t, uint64_t,  &user_role_t::by_contr>>
>;


} // namespace flon