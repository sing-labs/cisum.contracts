#pragma once
#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <string>
#include <set>

#include "flon.auth.db.hpp"

using namespace eosio;
using std::string;

namespace flon {

class [[eosio::contract("flon.auth")]] flonauth : public contract {
public:
    using contract::contract;

    flonauth(eosio::name receiver, eosio::name code, datastream<const char*> ds)
    : contract(receiver, code, ds),
      _global(get_self(), get_self().value)
    {
        _gstate = _global.exists() ? _global.get() : global_t{};
    }

    ~flonauth() {
        _global.set(_gstate, get_self());
    }

    // ========= 系统配置 =========
    ACTION init(const name& admin);
    ACTION setadmin(const name& new_admin);
    ACTION addallowlist(const name& acct);
    ACTION delallowlist(const name& acct);

    // ========= 角色 & 权限 =========
    ACTION addrole(const name& submitter, const name& role, const string& desc);
    ACTION delrole(const name& submitter, const name& role);

    ACTION grantrole(const name& granter, const name& user, const name& role);
    ACTION revokerole(const name& granter, const name& user, const name& role);

    ACTION addroleperm(const name& submitter,
                        const name& role,
                        const std::set<string>& perms,
                        const string& desc);

    ACTION delroleperm(const name& submitter,
                        const name& role,
                        const std::set<string>& perms);

    ACTION checkrole(const name& submitter,
                        const name& user,
                        const std::string& perm);

    // ========= Aliases =========
    using addrole_action    = eosio::action_wrapper<"addrole"_n,    &flonauth::addrole>;
    using delrole_action    = eosio::action_wrapper<"delrole"_n,    &flonauth::delrole>;
    using grantrole_action  = eosio::action_wrapper<"grantrole"_n,  &flonauth::grantrole>;
    using revokerole_action = eosio::action_wrapper<"revokerole"_n, &flonauth::revokerole>;
    using checkrole_action  = eosio::action_wrapper<"checkrole"_n,  &flonauth::checkrole>;

private:
    global_singleton _global;
    global_t         _gstate;

private:
    /// 权限检查：允许 self / admin / allowlist / 具备 perm 的 submitter
    void require_manage_perm(const name& submitter, const std::string& perm);

    /// 用户是否已拥有某个角色
    bool has_user_role(const name& user, const name& role) const;

    /// 角色是否存在
    bool role_exists(const name& role) const;
};

} // namespace flon