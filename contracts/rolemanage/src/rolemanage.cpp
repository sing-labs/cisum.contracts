#include "rolemanage.hpp"

using namespace eosio;
using namespace flon;


void rolemanage::init(const name& admin) {
    // 仅第一次允许由合约自身初始化
    require_auth(get_self());
    CHECKC(!_global.exists() || _gstate.admin.value == 0, err::INVALID_FORMAT, "already initialized");
    CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin not exist");
    _gstate.admin = admin;
}

void rolemanage::setadmin(const name& new_admin) {
    require_auth(_gstate.admin);
    CHECKC(is_account(new_admin), err::ACCOUNT_INVALID, "new admin not exist");
    _gstate.admin = new_admin;
}

void rolemanage::addoracle(const name& acct) {
    require_auth(_gstate.admin);
    CHECKC(is_account(acct), err::ACCOUNT_INVALID, "oracle not exist");
    _gstate.oracles.insert(acct);
}

void rolemanage::deloracle(const name& acct) {
    require_auth(_gstate.admin);
    auto it = _gstate.oracles.find(acct);
    CHECKC(it != _gstate.oracles.end(), err::RECORD_NOT_FOUND, "oracle not found");
    _gstate.oracles.erase(it);
}

void rolemanage::setrole(const name& role, const std::string& desc) {
    require_admin_or_oracle(_gstate, get_self());

    CHECKC(role.value != 0, err::INVALID_FORMAT, "role is empty");

    roles_idx roles(get_self(), get_self().value);
    auto byrole = roles.get_index<"byrole"_n>();
    auto it     = byrole.find(role.value);

    if (it == byrole.end()) {
        roles.emplace(get_self(), [&](auto& r){
            r.id         = roles.available_primary_key();
            r.role       = role;
            r.desc       = desc;
            r.created_at = current_time_point();
        });
    } else {
        byrole.modify(it, get_self(), [&](auto& r){
            r.desc = desc;
        });
    }
}

void rolemanage::delrole(const name& role) {
    require_admin_or_oracle(_gstate, get_self());

    roles_idx roles(get_self(), get_self().value);
    auto byrole = roles.get_index<"byrole"_n>();
    auto rit    = byrole.find(role.value);
    CHECKC(rit != byrole.end(), err::ROLE_NOT_FOUND, "role not found");

    userroles_idx ur(get_self(), get_self().value);
    auto ix = ur.get_index<"byrole"_n>();
    CHECKC(ix.find(role.value) == ix.end(), err::INVALID_FORMAT,
           "cannot delete role: still granted to some users");

    byrole.erase(rit);
}

void rolemanage::grantrole(const name& contract, const name& granter, const name& user, const name& role) {
    check(contract.value != 0, "[[1]] contract is empty");
    check(is_account(contract), "[[1a]] contract not exist");
    check(user.value     != 0, "[[2]] user is empty");
    check(role.value     != 0, "[[3]] role is empty");
    check(is_account(user),    "[[4]] user not exist");

    check(has_auth(granter),   "[[5]] granter signature required");
    require_admin_or_oracle(_gstate, get_self());

    userroles_idx ur(get_self(), get_self().value);

    auto by_uc = ur.get_index<"byusercontr"_n>();
    const uint128_t k_uc = ((uint128_t)user.value << 64) | contract.value;
    auto it = by_uc.lower_bound(k_uc);
    for (; it != by_uc.end() && it->by_user_contr() == k_uc; ++it) {
        if (it->role == role) return; // 幂等
    }

    ur.emplace(get_self(), [&](auto& r){
        r.id         = ur.available_primary_key();
        r.contract   = contract;
        r.user       = user;
        r.role       = role;
        r.granter    = granter;
        r.created_at = current_time_point();
    });
}

void rolemanage::revokerole(const name& contract, const name& granter, const name& user, const name& role) {
    check(contract.value != 0, "[[21]] contract is empty");
    check(user.value     != 0, "[[22]] user is empty");
    check(role.value     != 0, "[[23]] role is empty");

    require_auth(granter);
    require_admin_or_oracle(_gstate, get_self());

    userroles_idx ur(get_self(), get_self().value);
    auto by_uc = ur.get_index<"byusercontr"_n>();
    const uint128_t k_uc = ((uint128_t)user.value << 64) | contract.value;

    auto it = by_uc.lower_bound(k_uc);
    for (; it != by_uc.end() && it->by_user_contr() == k_uc; ) {
        if (it->role == role) {
            it = by_uc.erase(it);
            return;
        } else {
            ++it;
        }
    }
    check(false, " grant not found under this contract");
}

void rolemanage::checkrole(const name& contract, const name& user, const name& role) {
    check(contract.value != 0, "[[11]] contract is empty");
    check(is_account(contract), "[[11a]] contract not exist");
    check(user.value     != 0, "[[12]] user is empty");
    check(role.value     != 0, "[[13]] role is empty");

    bool ok = has_role(get_self(), user, role, contract);
    check(ok, " user has no such role under this contract");
}