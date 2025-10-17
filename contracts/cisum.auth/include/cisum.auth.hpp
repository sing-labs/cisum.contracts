#pragma once
#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <string>
#include <set>

#include "cisum.auth.db.hpp"

using namespace eosio;
using std::string;

namespace flon {

class [[eosio::contract("cisum.auth")]] cisumauth : public contract {
public:
    using contract::contract;

    cisumauth(eosio::name receiver, eosio::name code, datastream<const char*> ds)
    : contract(receiver, code, ds),
      _global(get_self(), get_self().value)
    {
        _gstate = _global.exists() ? _global.get() : global_t{};
    }

    ~cisumauth() {
        _global.set(_gstate, get_self());
    }

    // ========= 系统配置 =========
    ACTION init(const name& admin);
    ACTION setadmin(const name& new_admin);
    ACTION addallowlist(const name& acct);
    ACTION delallowlist(const name& acct);

    // ========= 角色 & 权限 =========
    ACTION addrole(const name& submitter, const string& role, const string& desc);
    ACTION delrole(const name& submitter, const string& role);

    ACTION grantrole(const name& granter, const name& user, const string& role);
    ACTION revokerole(const name& granter, const name& user, const string& role);

    ACTION addroleperm(const name& submitter,
                       const string& role,
                       const std::set<string>& perms,
                       const string& desc);

    ACTION delroleperm(const name& submitter,
                       const string& role,
                       const std::set<string>& perms);

    ACTION checkrole(const name& submitter,
                     const name& user,
                     const std::string& perm);

    // ========= Aliases =========
    using addrole_action    = eosio::action_wrapper<"addrole"_n,    &cisumauth::addrole>;
    using delrole_action    = eosio::action_wrapper<"delrole"_n,    &cisumauth::delrole>;
    using grantrole_action  = eosio::action_wrapper<"grantrole"_n,  &cisumauth::grantrole>;
    using revokerole_action = eosio::action_wrapper<"revokerole"_n, &cisumauth::revokerole>;
    using checkrole_action  = eosio::action_wrapper<"checkrole"_n,  &cisumauth::checkrole>;

private:
    global_singleton _global;
    global_t         _gstate;

private:
    /// 权限检查：允许 self / admin / allowlist / 具备 perm 的 submitter
    void require_manage_perm(const name& submitter, const std::string& perm);

    /// 用户是否已拥有某个角色
    bool has_user_role(const name& user, const std::string& role) const;

    /// 角色是否存在
    bool role_exists(const std::string& role) const;
};

} // namespace flon