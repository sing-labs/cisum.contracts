#include "cisum.auth.hpp"

using namespace eosio;
using namespace flon;

// 权限检查（允许合约自身 / admin / allowlist / 拥有某角色权限的 submitter）
void cisumauth::require_manage_perm(const name& submitter,
                                   const std::string& perm) {
    if (has_auth(get_self())) return;
    if (has_auth(_gstate.admin) && submitter == _gstate.admin) return;
    if (_gstate.allowlist.find(submitter) != _gstate.allowlist.end()) {
        require_auth(submitter);
        return;
    }

    require_auth(submitter);
    cisumauth::checkrole_action checkrole(get_self(), { {get_self(), "active"_n} });
    checkrole.send(get_self(), submitter, perm);
}

// 是否存在指定用户角色
bool cisumauth::has_user_role(const name& user, const std::string& role) const {
    userroles_idx ur(get_self(), get_self().value);
    auto by_userrole = ur.get_index<"byuserrole"_n>();
    return by_userrole.find(hash_u64_str(user.value, role)) != by_userrole.end();
}

// 是否存在指定角色
bool cisumauth::role_exists(const std::string& role) const {
    roles_idx roles(get_self(), get_self().value);
    auto byrole = roles.get_index<"byrole"_n>();
    return byrole.find(hash_str(role)) != byrole.end();
}

void cisumauth::init(const name& admin) {
    require_auth(get_self());
    CHECKC(!_global.exists() || _gstate.admin.value == 0,
           err::INVALID_FORMAT, "already initialized");
    CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin not exist");
    _gstate.admin = admin;
}

void cisumauth::setadmin(const name& new_admin) {
    require_auth(_gstate.admin);
    CHECKC(is_account(new_admin), err::ACCOUNT_INVALID, "new admin not exist");
    _gstate.admin = new_admin;
}

void cisumauth::addallowlist(const name& acct) {
    require_auth(_gstate.admin);
    CHECKC(is_account(acct), err::ACCOUNT_INVALID, "allowlist account not exist");
    _gstate.allowlist.insert(acct);
}

void cisumauth::delallowlist(const name& acct) {
    require_auth(_gstate.admin);
    auto it = _gstate.allowlist.find(acct);
    CHECKC(it != _gstate.allowlist.end(), err::RECORD_NOT_FOUND, "allowlist not found");
    _gstate.allowlist.erase(it);
}

void cisumauth::addrole(const name& submitter,
                       const std::string& role,
                       const std::string& desc) {
    require_manage_perm(submitter, "roleManage");
    CHECKC(!role.empty(), err::INVALID_FORMAT, "role is empty");

    roles_idx roles(get_self(), get_self().value);
    auto byrole = roles.get_index<"byrole"_n>();
    auto it     = byrole.find(hash_str(role));

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

void cisumauth::delrole(const name& submitter,
                       const std::string& role) {
    require_manage_perm(submitter, "roleManage");
    CHECKC(!role.empty(), err::INVALID_FORMAT, "role is empty");

    roles_idx roles(get_self(), get_self().value);
    auto byrole = roles.get_index<"byrole"_n>();
    auto rit    = byrole.find(hash_str(role));
    if (rit == byrole.end()) return;

    // 如果 role 还被用户持有，拒绝删除
    userroles_idx ur(get_self(), get_self().value);
    auto ur_byrole = ur.get_index<"byrole"_n>();
    CHECKC(ur_byrole.find(hash_str(role)) == ur_byrole.end(),
           err::STATUS_MISMATCH, "role is still assigned to users");

    // 清理 roleperms
    roleperms_idx perms_tbl(get_self(), get_self().value);
    auto rp_byrole = perms_tbl.get_index<"byrole"_n>();
    auto it = rp_byrole.find(hash_str(role));
    while (it != rp_byrole.end() && hash_str(it->role) == hash_str(role)) {
        it = rp_byrole.erase(it);
    }

    byrole.erase(rit);
}

void cisumauth::grantrole(const name& granter,
                         const name& user,
                         const std::string& role) {
    check(user.value != 0, "user is empty");
    check(!role.empty(), "role is empty");
    check(is_account(user), "user not exist");

    require_manage_perm(granter, "userManage");

    check(role_exists(role), "role not found: " + role);
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

void cisumauth::revokerole(const name& granter,
                          const name& user,
                          const std::string& role) {
    check(user.value != 0, "user is empty");
    check(!role.empty(), "role is empty");

    require_manage_perm(granter, "userManage");

    userroles_idx ur(get_self(), get_self().value);
    auto by_userrole = ur.get_index<"byuserrole"_n>();
    auto key = hash_u64_str(user.value, role);
    auto it = by_userrole.find(key);
    check(it != by_userrole.end(),
          "role " + role + " not found for user " + user.to_string());

    by_userrole.erase(it);
}

void cisumauth::checkrole(const name& submitter,
                         const name& user,
                         const std::string& perm) {
    require_auth(submitter);

    check(user.value != 0, "user is empty");
    check(!perm.empty(), "perm is empty");

    userroles_idx ur(get_self(), get_self().value);
    auto by_user = ur.get_index<"byuser"_n>();
    auto itr = by_user.find(user.value);

    bool ok = false;
    for (; itr != by_user.end() && itr->user == user; ++itr) {
        roleperms_idx perms_tbl(get_self(), get_self().value);
        auto byroleperm = perms_tbl.get_index<"byroleperm"_n>();
        auto rp_key = hash_two_u64_str(0, 0, itr->role + "|" + perm);
        if (byroleperm.find(rp_key) != byroleperm.end()) {
            ok = true;
            break;
        }
    }

    check(ok, "user " + user.to_string() +
              " does not have required permission: " + perm);
}

void cisumauth::addroleperm(const name& submitter,
                           const std::string& role,
                           const std::set<std::string>& perms,
                           const std::string& desc) {
    require_manage_perm(submitter, "roleManage");
    CHECKC(!role.empty(), err::INVALID_FORMAT, "role cannot be empty");
    CHECKC(!perms.empty(), err::INVALID_FORMAT, "perms cannot be empty");

    check(role_exists(role), "role not found: " + role);

    roleperms_idx perms_tbl(get_self(), get_self().value);
    auto byroleperm = perms_tbl.get_index<"byroleperm"_n>();

    for (const auto& p : perms) {
        CHECKC(!p.empty(), err::INVALID_FORMAT, "perm cannot be empty");
        auto rp_key = hash_two_u64_str(0, 0, role + "|" + p);
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

void cisumauth::delroleperm(const name& submitter,
                           const std::string& role,
                           const std::set<std::string>& perms) {
    require_manage_perm(submitter, "roleManage");
    CHECKC(!role.empty(), err::INVALID_FORMAT, "role cannot be empty");
    CHECKC(!perms.empty(), err::INVALID_FORMAT, "perms cannot be empty");

    roleperms_idx perms_tbl(get_self(), get_self().value);
    auto byroleperm = perms_tbl.get_index<"byroleperm"_n>();

    for (const auto& p : perms) {
        auto rp_key = hash_two_u64_str(0, 0, role + "|" + p);
        auto it = byroleperm.find(rp_key);
        if (it != byroleperm.end()) {
            byroleperm.erase(it);
        }
    }
}