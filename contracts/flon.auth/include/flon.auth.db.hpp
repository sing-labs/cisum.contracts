#pragma once
#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <eosio/crypto.hpp>   // <- 需要 sha256 / checksum256
#include <set>
#include <string>
#include <vector>

using namespace eosio;
using std::string;

namespace flon {

// ---------- 工具：打包并哈希 ----------
inline checksum256 hash_bytes(const void* data, size_t size) {
    return sha256(static_cast<const char*>(data), size);
}
inline checksum256 hash_str(const std::string& s) {
    return sha256(s.data(), s.size());
}
template <typename T>
inline void append_bytes(std::vector<char>& buf, const T& v) {
    const char* p = reinterpret_cast<const char*>(&v);
    buf.insert(buf.end(), p, p + sizeof(T));
}
inline checksum256 hash_u64_str(uint64_t v, const std::string& s) {
    std::vector<char> buf;
    buf.reserve(sizeof(uint64_t) + s.size());
    append_bytes(buf, v);
    buf.insert(buf.end(), s.begin(), s.end());
    return sha256(buf.data(), buf.size());
}
inline checksum256 hash_two_u64_str(uint64_t a, uint64_t b, const std::string& s) {
    std::vector<char> buf;
    buf.reserve(sizeof(uint64_t)*2 + s.size());
    append_bytes(buf, a);
    append_bytes(buf, b);
    buf.insert(buf.end(), s.begin(), s.end());
    return sha256(buf.data(), buf.size());
}

// -------- 错误码/宏（原样保留） --------
enum class err: uint8_t {
   INVALID_FORMAT     = 0,
   RECORD_NOT_FOUND   = 1,
   ROLE_EXISTS        = 2,
   ROLE_NOT_FOUND     = 3,
   GRANT_EXISTS       = 4,
   GRANT_NOT_FOUND    = 5,
   ACCOUNT_INVALID    = 6,
   PERMISSION_DENIED  = 7,
   STATUS_MISMATCH    = 8
};
#define CHECKC(exp, code, msg) \
  { if (!(exp)) eosio::check(false, string("[[") + std::to_string((int)code) + string("]] ") + msg); }

// -------- 全局配置（原样保留） --------
struct [[eosio::table, eosio::contract("flon.auth")]] global_t {
  name           admin;
  std::set<name> allowlist;
  uint64_t       last_role_id=0;
  uint64_t       last_userrole_id=0;
  uint64_t       last_roleperm_id=0;
  EOSLIB_SERIALIZE(global_t, (admin)(allowlist)(last_role_id)(last_userrole_id)(last_roleperm_id))
};
using global_singleton = eosio::singleton<"global"_n, global_t>;

// scope:_self
struct [[eosio::table("roles"), eosio::contract("flon.auth")]] role_t {
    uint64_t    id;         // 自增主键
    name        role;       // 角色名（任意字符串）
    std::string desc;       // 描述
    time_point  created_at; // 创建时间

    uint64_t     primary_key()  const { return id; }
    uint64_t     byrole()       const { return role.value; }

    EOSLIB_SERIALIZE(role_t, (id)(role)(desc)(created_at))
};
using roles_idx = eosio::multi_index<
    "roles"_n, role_t,
    indexed_by<"byrole"_n, const_mem_fun<role_t, uint64_t, &role_t::byrole>>
>;

// -------- 表：用户角色绑定（role=string，各索引改为 checksum256 版本） --------
// scope:  _self
struct [[eosio::table("userroles"), eosio::contract("flon.auth")]] user_role_t {
    uint64_t    id;         // 自增主键
    name        user;       // 被授予用户
    name        role;       // 角色名（string）
    name        granter;    // 授权人
    time_point  created_at; // 授权时间

    uint64_t    primary_key()  const { return id; }
    uint64_t    by_user()      const { return user.value; }

    uint128_t   by_userrole()  const { return (uint128_t(user.value) << 64) | role.value; }
    uint64_t    by_role()      const { return role.value; }

    EOSLIB_SERIALIZE(user_role_t, (id)(user)(role)(granter)(created_at))
};

using userroles_idx = eosio::multi_index<
    "userroles"_n, user_role_t,
    indexed_by<"byuser"_n, const_mem_fun<user_role_t, uint64_t, &user_role_t::by_user>>,
    indexed_by<"byuserrole"_n, const_mem_fun<user_role_t, uint128_t, &user_role_t::by_userrole>>,
    indexed_by<"byrole"_n, const_mem_fun<user_role_t, uint64_t, &user_role_t::by_role>>
>;


// -------- 表：角色权限绑定 --------
// scope:  _self
struct [[eosio::table("roleperms"), eosio::contract("flon.auth")]] role_perm_t {
    uint64_t    id;         // 自增主键
    name        role;       // 角色名
    std::string perm;       // 权限名（string）
    std::string desc;       // 权限描述
    time_point  created_at; // 创建时间
    uint64_t    primary_key()   const { return id; }
    uint64_t    by_role()       const { return role.value; }
    checksum256 by_perm()       const { return hash_str(perm); }
    uint128_t by_roleperm() const {
            // 复合键 (role.value, hash(perm) 的前64bit)
            auto p = hash_str(perm);
            uint64_t perm_hash_prefix = *reinterpret_cast<const uint64_t*>(p.extract_as_byte_array().data());
            return (uint128_t(role.value) << 64) | perm_hash_prefix;
        }
    // 用 role+perm 拼接避免重复

    EOSLIB_SERIALIZE(role_perm_t, (id)(role)(perm)(desc)(created_at))
};

using roleperms_idx = eosio::multi_index<
    "roleperms"_n, role_perm_t,
    indexed_by<"byrole"_n, const_mem_fun<role_perm_t, uint64_t, &role_perm_t::by_role>>,
    indexed_by<"byperm"_n, const_mem_fun<role_perm_t, checksum256, &role_perm_t::by_perm>>,
    indexed_by<"byroleperm"_n, const_mem_fun<role_perm_t, uint128_t, &role_perm_t::by_roleperm>>
>;


} // namespace flon