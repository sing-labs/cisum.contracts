#include "flon.auth.hpp"

using namespace eosio;
using namespace flon;

void flonauth::init(const name& admin) {
    require_auth(get_self());
    CHECKC(!_global.exists() || _gstate.admin.value == 0, err::INVALID_FORMAT, "already initialized");
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
    CHECKC(is_account(acct), err::ACCOUNT_INVALID, "allowlist not exist");
    _gstate.allowlist.insert(acct);
}

void flonauth::delallowlist(const name& acct) {
    require_auth(_gstate.admin);
    auto it = _gstate.allowlist.find(acct);
    CHECKC(it != _gstate.allowlist.end(), err::RECORD_NOT_FOUND, "allowlist not found");
    _gstate.allowlist.erase(it);
}

void flonauth::setrole(const std::string& role, const std::string& desc) {
    require_admin_or_allowlist(_gstate, get_self());
    CHECKC(!role.empty(), err::INVALID_FORMAT, "role is empty");

    roles_idx roles(get_self(), get_self().value);
    auto byrole = roles.get_index<"byrole"_n>();
    auto it     = byrole.find(hash_str(role));

    if (it == byrole.end()) {
        roles.emplace(get_self(), [&](auto& r){
            r.id         = ++ _gstate.last_role_id;;
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

void flonauth::delrole(const std::string& role) {
    require_admin_or_allowlist(_gstate, get_self());

    roles_idx roles(get_self(), get_self().value);
    auto byrole = roles.get_index<"byrole"_n>();
    auto rit    = byrole.find(hash_str(role));
    CHECKC(rit != byrole.end(), err::ROLE_NOT_FOUND, "role not found");

    userroles_idx ur(get_self(), get_self().value);
    auto ix = ur.get_index<"byrole"_n>();
    CHECKC(ix.find(hash_str(role)) == ix.end(), err::INVALID_FORMAT,
           "cannot delete role: still granted to some users");

    byrole.erase(rit);
}

void flonauth::grantrole(const name& contract,
                           const name& granter,
                           const name& user,
                           const std::string& role) {
    if (contract.value != 0) {
        check(is_account(contract), "[[1a]] contract not exist");
    }
    check(user.value != 0, "[[2]] user is empty");
    check(!role.empty(), "[[3]] role is empty");
    check(is_account(user), "[[4]] user not exist");

    check(has_auth(granter), "[[5]] granter signature required");
    require_admin_or_allowlist(_gstate, get_self());

    userroles_idx ur(get_self(), get_self().value);
    auto by_uc = ur.get_index<"byusercontr"_n>();
    const uint128_t k_uc = ((uint128_t)user.value << 64) | contract.value;

    for (auto it = by_uc.lower_bound(k_uc);
         it != by_uc.end() && it->by_usercontr() == k_uc; ++it) {
        if (it->role == role) return; // 幂等
    }

    ur.emplace(get_self(), [&](auto& r){
        r.id         = ++ _gstate.last_userrole_id;
        r.contract   = contract;
        r.user       = user;
        r.role       = role;
        r.granter    = granter;
        r.created_at = current_time_point();
    });
}

void flonauth::revokerole(const name& contract,
                            const name& granter,
                            const name& user,
                            const std::string& role) {
    check(contract.value != 0, "[[21]] contract is empty");
    check(user.value     != 0, "[[22]] user is empty");
    check(!role.empty(), "[[23]] role is empty");

    require_auth(granter);
    require_admin_or_allowlist(_gstate, get_self());

    userroles_idx ur(get_self(), get_self().value);
    auto by_uc = ur.get_index<"byusercontr"_n>();
    const uint128_t k_uc = ((uint128_t)user.value << 64) | contract.value;

    auto it = by_uc.lower_bound(k_uc);
    for (; it != by_uc.end() && it->by_usercontr() == k_uc; ) {
        if (it->role == role) {
            it = by_uc.erase(it);
            return;
        } else {
            ++it;
        }
    }
    check(false, "grant not found under this contract");
}

void flonauth::checkrole(const name& submitter,
                    const name& contract,
                    const name& user,
                    const std::vector<std::string>& roles) {
    // 谁发起校验，必须签名（合约 / admin / allowlist）
    require_auth(submitter);
    check(is_allowlist(_gstate, submitter) || submitter == _gstate.admin || submitter == get_self(),
          "submitter not authorized (not admin/allowlist/self)");

    check(user.value != 0, "user is empty");
    check(!roles.empty(), "roles vector is empty");
    if (contract.value != 0) {
        check(is_account(contract), "contract not exist");
    }

    userroles_idx ur(get_self(), get_self().value);
    auto by_uc = ur.get_index<"byusercontr"_n>();
    const uint128_t k_uc = ((uint128_t)user.value << 64) | contract.value;

    bool found = false;
    for (auto it = by_uc.lower_bound(k_uc);
         it != by_uc.end() && it->by_usercontr() == k_uc; ++it) {
        for (const auto& r : roles) {
            if (it->role == r) {
                found = true;
                break;
            }
        }
        if (found) break;
    }

    check(found, "user has none of the required roles under this contract");
}