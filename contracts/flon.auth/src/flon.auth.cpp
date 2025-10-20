#include "flon.auth.hpp"

using namespace eosio;
using namespace flon;

// 权限检查（允许合约自身 / admin / allowlist / 拥有某角色权限的 submitter）
void flonauth::require_manage_perm(const name& submitter,
                                   const std::string& perm) {
    if (has_auth(get_self())) return;
    if (has_auth(_gstate.admin) && submitter == _gstate.admin) return;
    if (_gstate.allowlist.find(submitter) != _gstate.allowlist.end()) {
        require_auth(submitter);
        return;
    }

    require_auth(submitter);
    flonauth::checkrole_action checkrole(get_self(), { {get_self(), "active"_n} });
    checkrole.send(get_self(), submitter, perm);
}

// 是否存在指定用户角色
bool flonauth::has_user_role(const name& user, const name& role) const {
    userroles_idx ur(get_self(), get_self().value);
    auto by_userrole = ur.get_index<"byuserrole"_n>();
    uint128_t key = (uint128_t(user.value) << 64) | role.value;
    return by_userrole.find(key) != by_userrole.end();
}

// 是否存在指定角色
bool flonauth::role_exists(const name& role) const {
    roles_idx roles(get_self(), get_self().value);
    auto byrole = roles.get_index<"byrole"_n>();
    return byrole.find(role.value) != byrole.end();
}

void flonauth::init(const name& admin) {
    require_auth(get_self());
    // CHECKC(!_global.exists() || _gstate.admin.value == 0,
    //        err::INVALID_FORMAT, "already initialized");
    CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin not exist");
    _gstate.admin = admin;
}

void flonauth::setadmin(const name& new_admin) {
    require_auth(_gstate.admin);
    CHECKC(is_account(new_admin), err::ACCOUNT_INVALID, "new admin not exist");
    _gstate.admin = new_admin;
}

void flonauth::addallowlist(const name& acct) {
    require_auth(_gstate.admin);
    CHECKC(is_account(acct), err::ACCOUNT_INVALID, "allowlist account not exist");
    _gstate.allowlist.insert(acct);
}

void flonauth::delallowlist(const name& acct) {
    require_auth(_gstate.admin);
    auto it = _gstate.allowlist.find(acct);
    CHECKC(it != _gstate.allowlist.end(), err::RECORD_NOT_FOUND, "allowlist not found");
    _gstate.allowlist.erase(it);
}

void flonauth::addrole(const name& submitter,
                       const name& role,
                       const std::string& desc) {
    require_manage_perm(submitter, "roleManage");
    CHECKC(role.value != 0, err::INVALID_FORMAT, "role is empty");

    roles_idx roles(get_self(), get_self().value);
    auto byrole = roles.get_index<"byrole"_n>();
    auto it     = byrole.find(role.value);

    if (it == byrole.end()) {
        roles.emplace(get_self(), [&](auto& r){
            r.id         = ++_gstate.last_role_id;
            r.role       = role;
            r.desc       = desc;
            r.created_at = current_time_point();
        });
    } else {
        byrole.modify(it, get_self(), [&](auto& r){
            r.desc       = desc;
            r.created_at = current_time_point();
        });
    }
}

void flonauth::delrole(const name& submitter, const name& role) {
    require_manage_perm(submitter, "roleManage");
    CHECKC(role.value != 0, err::INVALID_FORMAT, "role is empty");

    roles_idx roles(get_self(), get_self().value);
    auto byrole = roles.get_index<"byrole"_n>();
    auto role_it = byrole.find(role.value);
    if (role_it == byrole.end()) return;

    // 如果 role 还被用户持有，拒绝删除
    userroles_idx ur(get_self(), get_self().value);
    auto ur_byrole = ur.get_index<"byrole"_n>();
    CHECKC(ur_byrole.find(role.value) == ur_byrole.end(),
           err::STATUS_MISMATCH, "role is still assigned to users");

    // 清理 roleperms
    roleperms_idx perms_tbl(get_self(), get_self().value);
    auto rp_byrole = perms_tbl.get_index<"byrole"_n>();
    auto perm_it = rp_byrole.find(role.value);
    while (perm_it != rp_byrole.end() && perm_it->role == role) {
        perm_it = rp_byrole.erase(perm_it);
    }

    byrole.erase(role_it);
}

void flonauth::grantrole(const name& granter,
                         const name& user,
                         const name& role) {
    check(user.value != 0, "user is empty");
    check(role.value != 0, "role is empty");
    check(is_account(user), "user not exist");

    require_manage_perm(granter, "userManage");

    check(role_exists(role), "role not found: " + role.to_string());
    if (has_user_role(user, role)) return; // 幂等

    userroles_idx ur(get_self(), get_self().value);
    ur.emplace(get_self(), [&](auto& r){
        r.id         = ++_gstate.last_userrole_id;
        r.user       = user;
        r.role       = role;
        r.granter    = granter;
        r.created_at = current_time_point();
    });
}

void flonauth::revokerole(const name& granter,
                          const name& user,
                          const name& role) {
    check(user.value != 0, "user is empty");
    check(role.value != 0, "role is empty");

    require_manage_perm(granter, "userManage");

    userroles_idx ur(get_self(), get_self().value);
    auto by_userrole    = ur.get_index<"byuserrole"_n>();
    uint128_t key       = (uint128_t(user.value) << 64) | role.value;
    auto it = by_userrole.find(key);
    check(it != by_userrole.end(),
          std::string("role ") + role.to_string() + " not found for user " + user.to_string()  );

    by_userrole.erase(it);
}

void flonauth::checkrole(const name& submitter,
                         const name& user,
                         const std::string& perm) {
    require_auth(submitter);

    check(user.value != 0, "user is empty");
    check(!perm.empty(), "perm is empty");

    // 先查用户有哪些角色
    userroles_idx ur(get_self(), get_self().value);
    auto by_user = ur.get_index<"byuser"_n>();
    auto itr = by_user.find(user.value);

    bool ok = false;

    // 预计算 perm 的 hash 前缀（8 字节）
    auto p = hash_str(perm);
    uint64_t perm_hash_prefix =
        *reinterpret_cast<const uint64_t*>(p.extract_as_byte_array().data());

    // 遍历用户角色，检查是否有对应的权限
    roleperms_idx perms_tbl(get_self(), get_self().value);
    auto byroleperm = perms_tbl.get_index<"byroleperm"_n>();

    for (; itr != by_user.end() && itr->user == user; ++itr) {
        uint128_t rp_key = (uint128_t(itr->role.value) << 64) | perm_hash_prefix;
        if (byroleperm.find(rp_key) != byroleperm.end()) {
            ok = true;
            break;
        }
    }

    check(ok, "user " + user.to_string() +
              " does not have required permission: " + perm);
}

void flonauth::addroleperm(const name& submitter,
                           const name& role,
                           const std::set<std::string>& perms,
                           const std::string& desc) {
    require_manage_perm(submitter, "roleManage");
    CHECKC(role.value != 0, err::INVALID_FORMAT, "role cannot be empty");
    CHECKC(!perms.empty(), err::INVALID_FORMAT, "perms cannot be empty");

    check(role_exists(role), "role not found: " + role.to_string());

    roleperms_idx perms_tbl(get_self(), get_self().value);
    auto byroleperm = perms_tbl.get_index<"byroleperm"_n>();

    for (const auto& p : perms) {
        CHECKC(!p.empty(), err::INVALID_FORMAT, "perm cannot be empty");

        // 🔹 hash 权限字符串
        auto h = hash_str(p);
        uint64_t perm_hash_prefix =
            *reinterpret_cast<const uint64_t*>(h.extract_as_byte_array().data());

        // 🔸 构建复合 key
        uint128_t rp_key = (uint128_t(role.value) << 64) | perm_hash_prefix;

        // 🔸 检查是否已存在
        CHECKC(byroleperm.find(rp_key) == byroleperm.end(),
               err::GRANT_EXISTS, "permission already granted: " + p);

        perms_tbl.emplace(get_self(), [&](auto& r) {
            r.id         = ++_gstate.last_roleperm_id;
            r.role       = role;
            r.perm       = p;
            r.desc       = desc;
            r.created_at = current_time_point();
        });
    }
}

void flonauth::delroleperm(const name& submitter,
                           const name& role,
                           const std::set<std::string>& perms) {
    require_manage_perm(submitter, "roleManage");
    CHECKC(role.value != 0, err::INVALID_FORMAT, "role cannot be empty");
    CHECKC(!perms.empty(), err::INVALID_FORMAT, "perms cannot be empty");

    roleperms_idx perms_tbl(get_self(), get_self().value);
    auto byroleperm = perms_tbl.get_index<"byroleperm"_n>();

    for (const auto& p : perms) {
        auto h = hash_str(p);
        uint64_t perm_hash_prefix =
            *reinterpret_cast<const uint64_t*>(h.extract_as_byte_array().data());

        uint128_t rp_key = (uint128_t(role.value) << 64) | perm_hash_prefix;

        auto it = byroleperm.find(rp_key);
        if (it != byroleperm.end()) {
            byroleperm.erase(it);
        }
    }
}
