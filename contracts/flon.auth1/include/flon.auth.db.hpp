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

// -------- 错误码/宏 --------
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

// -------- 全局配置 --------
struct [[eosio::table, eosio::contract("flon.auth")]] global_t {
  name           admin;
  std::set<name> allowlist;
  uint64_t       last_role_id=0;
  uint64_t       last_userrole_id=0;
  uint64_t       last_roleperm_id=0;
  EOSLIB_SERIALIZE(global_t, (admin)(allowlist)(last_role_id)(last_userrole_id)(last_roleperm_id))
};
using global_singleton = eosio::singleton<"globals"_n, global_t>;

// -------- 表：roles --------
struct [[eosio::table("roles"), eosio::contract("flon.auth")]] role_t {
    uint64_t    id;
    std::string role;       // ✅ 改为 string
    std::string desc;
    time_point  created_at;

    uint64_t     primary_key()  const { return id; }
    checksum256  byrole()       const { return hash_str(role); }

    EOSLIB_SERIALIZE(role_t, (id)(role)(desc)(created_at))
};

using roles_idx = eosio::multi_index<
    "roles"_n, role_t,
    indexed_by<"byrole"_n, const_mem_fun<role_t, checksum256, &role_t::byrole>>
>;

// -------- 表：userroles --------
struct [[eosio::table("userroles"), eosio::contract("flon.auth")]] user_role_t {
    uint64_t    id;
    name        user;
    std::string role;       // ✅ 改为 string
    name        granter;
    time_point  created_at;

    uint64_t    primary_key()  const { return id; }
    uint64_t    by_user()      const { return user.value; }

    checksum256 by_userrole()  const { return hash_u64_str(user.value, role); }
    checksum256 by_role()      const { return hash_str(role); }

    EOSLIB_SERIALIZE(user_role_t, (id)(user)(role)(granter)(created_at))
};

using userroles_idx = eosio::multi_index<
    "userroles"_n, user_role_t,
    indexed_by<"byuser"_n, const_mem_fun<user_role_t, uint64_t, &user_role_t::by_user>>,
    indexed_by<"byuserrole"_n, const_mem_fun<user_role_t, checksum256, &user_role_t::by_userrole>>,
    indexed_by<"byrole"_n, const_mem_fun<user_role_t, checksum256, &user_role_t::by_role>>
>;

// -------- 表：roleperms --------
struct [[eosio::table("roleperms"), eosio::contract("flon.auth")]] role_perm_t {
    uint64_t    id;
    std::string role;       // ✅ 改为 string
    std::string perm;
    std::string desc;
    time_point  created_at;

    uint64_t    primary_key()   const { return id; }
    checksum256 by_role()       const { return hash_str(role); }
    checksum256 by_perm()       const { return hash_str(perm); }
    checksum256 by_roleperm()   const { return hash_two_u64_str(0, 0, role + "|" + perm); }

    EOSLIB_SERIALIZE(role_perm_t, (id)(role)(perm)(desc)(created_at))
};

using roleperms_idx = eosio::multi_index<
    "roleperms"_n, role_perm_t,
    indexed_by<"byrole"_n,     const_mem_fun<role_perm_t, checksum256, &role_perm_t::by_role>>,
    indexed_by<"byperm"_n,     const_mem_fun<role_perm_t, checksum256, &role_perm_t::by_perm>>,
    indexed_by<"byroleperm"_n, const_mem_fun<role_perm_t, checksum256, &role_perm_t::by_roleperm>>
>;

} // namespace flon