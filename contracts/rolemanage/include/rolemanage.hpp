#pragma once
#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <string>

#include "rolemanage.db.hpp"

using namespace eosio;
using std::string;

namespace flon {

class [[eosio::contract("rolemanage")]] rolemanage : public contract {
public:
    using contract::contract;

    rolemanage(eosio::name receiver, eosio::name code, datastream<const char*> ds)
    : contract(receiver, code, ds),
      _global(get_self(), get_self().value)
    {
      _gstate = _global.exists() ? _global.get() : global_t{};
    }

    ~rolemanage() { _global.set(_gstate, get_self()); }


    ACTION init(const name& admin);
    ACTION setadmin(const name& new_admin);
    ACTION addoracle(const name& acct);
    ACTION deloracle(const name& acct);
    // ========= Actions =========

    /**
     * setrole - 新建/更新一个角色（纯配置）
     * 权限：默认仅合约自身（可按需调整）
     */
    ACTION setrole(const name& role, const string& desc);

    /**
     * delrole - 删除一个角色（需确保无人持有该角色）
     * 权限：默认仅合约自身
     */
    ACTION delrole(const name& role);

    /**
     * grantrole - 给用户授予某个角色（(user,role) 幂等）
     * 权限：由 granter 自签（可在实现中增加 granter 资格校验）
     */
    ACTION grantrole(const name& contract, const name& granter, const name& user, const name& role);

    /**
     * revokerole - 撤销用户的某个角色
     * 权限：默认仅原授权人或合约自身
     */
    ACTION revokerole(const name& contract, const name& granter, const name& user, const name& role);

    /**
     * checkrole - 只读校验（供业务合约内联调用）
     * 若 user 不具备 role，则抛错
     */
    ACTION checkrole(const name& contract, const name& user, const name& role);

    // ========= Aliases =========
    using setrole_action    = eosio::action_wrapper<"setrole"_n,    &rolemanage::setrole>;
    using delrole_action    = eosio::action_wrapper<"delrole"_n,    &rolemanage::delrole>;
    using grantrole_action  = eosio::action_wrapper<"grantrole"_n,  &rolemanage::grantrole>;
    using revokerole_action = eosio::action_wrapper<"revokerole"_n, &rolemanage::revokerole>;
    using checkrole_action  = eosio::action_wrapper<"checkrole"_n,  &rolemanage::checkrole>;

private:
    global_singleton _global;
    global_t         _gstate;


private:

    inline bool is_admin(const global_t& g, name acct) {
        return g.admin.value && (acct == g.admin);
    }

    inline bool is_oracle(const global_t& g, name acct) {
        return g.oracles.find(acct) != g.oracles.end();
    }
    inline bool has_any_oracle_auth(const global_t& g) {
        for (const auto& o : g.oracles) {
            if (has_auth(o)) return true;
        }
        return false;
    }

    inline void require_admin_or_oracle(const global_t& g, name self) {
        if (g.admin.value && has_auth(g.admin)) return;  // admin 签名
        if (has_auth(self)) return;                      // 合约自身签名（部署/迁移时好用）
        if (has_any_oracle_auth(g)) return;              // 任一 oracle 签名
        check(false, "[[7]] not admin/oracle");
    }

    inline bool has_role(name self_contract, name user, name role, name contract) {
        userroles_idx ur(self_contract, self_contract.value);
        auto idx = ur.get_index<"byusercontr"_n>();
        const uint128_t key = ((uint128_t)user.value << 64) | contract.value;
        auto itr = idx.lower_bound(key);
        // 扫描该 (user,contract) 分组内是否存在相同 role
        for (; itr != idx.end() && itr->by_user_contr() == key; ++itr) {
            if (itr->role == role) return true;
        }
        return false;
    }

};

} // namespace flon