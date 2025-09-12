#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <string>

#include "poh.cisum.hpp"
#include "flon.token.hpp"

namespace flon {

using eosio::asset;
using eosio::check;
using eosio::current_time_point;
using eosio::is_account;
using eosio::name;
using std::string;

void poh_cisum::init(name platform, name registrar, asset max_issued) {
    require_auth(get_self());

    CHECKC(is_account(platform),  err::ACCOUNT_INVALID, "platform not exist");
    CHECKC(is_account(registrar), err::ACCOUNT_INVALID, "registrar not exist");
    CHECKC(max_issued.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "max_issued must be CISUM");
    CHECKC(max_issued.amount > 0,          err::NOT_POSITIVE,    "max_issued must be positive");
    CHECKC(max_issued.amount >= _gstate.cisum_issued.amount,
           err::EXCEED_LIMIT, "max_issued must be greater than current issued");
    _gstate.platform_acct           = platform;
    _gstate.registrar               = registrar;
    _gstate.max_issued              = max_issued;

    _global.set(_gstate, get_self());
}

void poh_cisum::setmaxissued(asset max_issued) {
    require_auth(get_self());
    CHECKC(max_issued.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "max_issued must be CISUM");
    CHECKC(max_issued.amount > 0,          err::NOT_POSITIVE,    "max_issued must be positive");
    CHECKC(max_issued.amount >= _gstate.cisum_issued.amount,
           err::EXCEED_LIMIT, "max_issued must be greater than current issued");

    _gstate.max_issued              = max_issued;
    _global.set(_gstate, get_self());
}


void poh_cisum::setplatform(name platform) {
    require_auth(get_self());
    CHECKC(is_account(platform), err::ACCOUNT_INVALID, "platform not exist");
    _gstate.platform_acct = platform;
    _global.set(_gstate, get_self());
}

void poh_cisum::setregistrar(name registrar) {
    require_auth(get_self());
    CHECKC(is_account(registrar), err::ACCOUNT_INVALID, "registrar not exist");
    _gstate.registrar = registrar;
    _global.set(_gstate, get_self());
}

// -------------------- 业务动作 --------------------

void poh_cisum::registreward(name user, string memo) {
    // 基本校验
    CHECKC(_gstate.registrar.value != 0, err::RECORD_NO_FOUND, "registrar not set");
    require_auth(_gstate.registrar);

    CHECKC(is_account(user), err::ACCOUNT_INVALID, "user not exist");
    CHECKC(memo.size() <= 256, err::INVALID_FORMAT, "memo too long");
    CHECKC(_gstate.platform_acct.value != 0, err::RECORD_NO_FOUND, "platform not set");

    // USDT -> CISUM
    asset usdt = _gstate.usdt_per_user;
    CHECKC(usdt.symbol == USDT_SYM, err::SYMBOL_MISMATCH, "usdt_per_user symbol mismatch");
    CHECKC(usdt.amount > 0,         err::NOT_POSITIVE,    "usdt_per_user not positive");

    asset reward = usdt_to_cisum(usdt);
    CHECKC(reward.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "reward symbol mismatch");
    CHECKC(reward.amount > 0,          err::NOT_POSITIVE,    "reward too small");

    bool can_issue_cisum = (_gstate.max_issued.amount > 0) &&
                           (_gstate.cisum_issued.amount + reward.amount <= _gstate.max_issued.amount);

    // ===== CISUM：若额度足够则发放，否则跳过 =====
    if (can_issue_cisum) {
        ISSUE(
          CISUM_BANK,
          _self,
          reward,
          std::string("poh cisum mint: ") + user.to_string() + " | " + memo
        );
        TRANSFER(
          CISUM_BANK,
          _gstate.platform_acct,
          reward,
          std::string("poh reward: ") + user.to_string() + " | " + memo + " ;amount: " + reward.to_string()
        );
        _gstate.cisum_issued += reward;
    }

    // ===== NESTAR：总是发放 =====
    ISSUE(
      NESTAR_BANK,
      _self,
      NESTAR_BONUS,
      std::string("poh nestar mint: ") + user.to_string() + " ;amount: " + NESTAR_BONUS.to_string()
    );
    TRANSFER(
      NESTAR_BANK,
      user,
      NESTAR_BONUS,
      std::string("poh bonus: ") + user.to_string() + " ;amount: " + NESTAR_BONUS.to_string()
    );
    _gstate.nestar_issued += NESTAR_BONUS;

    _global.set(_gstate, get_self());
}

} // namespace flon