#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <string>

#include "poh.cisum.hpp"
#include "flon/flon.token.hpp"
#include "poe.cisum.db.hpp"
#include "poe.cisum.hpp"
#include "flon/consts.hpp"
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
    CHECKC(max_issued.symbol == SING_SYM, err::SYMBOL_MISMATCH, "max_issued must be SING");
    CHECKC(max_issued.amount > 0,          err::NOT_POSITIVE,    "max_issued must be positive");
    CHECKC(max_issued.amount >= _gstate.sing_issued.amount,
           err::EXCEED_LIMIT, "max_issued must be greater than current issued");
    _gstate.platform_acct           = platform;
    _gstate.registrar               = registrar;
    _gstate.max_issued              = max_issued;
    _global.set(_gstate, get_self());
}


void poh_cisum::setmaxissued(asset max_issued) {
    require_auth(get_self());
    CHECKC(max_issued.symbol == SING_SYM, err::SYMBOL_MISMATCH, "max_issued must be SING");
    CHECKC(max_issued.amount > 0,          err::NOT_POSITIVE,    "max_issued must be positive");
    CHECKC(max_issued.amount >= _gstate.sing_issued.amount,
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

void poh_cisum::notifyreward(const name&  from,
                                const name&       to,
                                const asset&      award_amount,
                                const string&     memo,
                                const name&       reward_type,
                                const string&     reward_ref_id,
                                const uint64_t&   created_at)
{
     require_auth(get_self());
}


void poh_cisum::registreward(const name& submitter,
                             const name& inviter,   // 可为空：inviter.value == 0 表示无邀请人
                             const name& invitee)
{

    CHECKC(_gstate.registrar.value != 0,        err::RECORD_NO_FOUND, "registrar not set");
    CHECKC(is_account(_gstate.registrar),       err::ACCOUNT_INVALID, "registrar not exist");
    CHECKC(is_account(invitee),                 err::ACCOUNT_INVALID, "invitee not exist");

    CHECKC(submitter == _gstate.registrar,      err::DID_NOT_AUTH,    "submitter must be registrar");
    require_auth(submitter);

    if (inviter.value != 0) {
        CHECKC(is_account(inviter),             err::ACCOUNT_INVALID, "inviter not exist");
        CHECKC(inviter != invitee,              err::INVALID_FORMAT,  "inviter cannot equal invitee");
    }

    CHECKC(_gstate.platform_acct.value != 0,    err::RECORD_NO_FOUND, "platform not set");

    // ===== 主奖励：USDT -> SING 给平台（平台再按你的业务分配），CISUM 给 invitee =====
    //  USDT -> SING 给平台账户
    {
        asset usdt = _gstate.usdt_per_user;
        CHECKC(usdt.symbol == USDT_SYM,         err::SYMBOL_MISMATCH, "usdt_per_user symbol mismatch");
        CHECKC(usdt.amount > 0,                 err::NOT_POSITIVE,    "usdt_per_user not positive");

        asset reward_sing = exchange_asset(usdt);
        CHECKC(reward_sing.symbol == SING_SYM,err::SYMBOL_MISMATCH, "reward_sing symbol mismatch");
        CHECKC(reward_sing.amount > 0,         err::NOT_POSITIVE,    "reward_sing too small");

        // 额度控制：仅在未超上限时铸造 SING
        bool can_issue_sing = (_gstate.max_issued.amount > 0) &&
                               (_gstate.sing_issued.amount + reward_sing.amount <= _gstate.max_issued.amount);

        if (can_issue_sing) {
            ISSUE(
                SING_BANK,
                _self,
                reward_sing,
                std::string("NewReg Reward:")+ invitee.to_string()
            );
            TRANSFER(
                SING_BANK,
                _gstate.platform_acct,
                reward_sing,
                std::string("NewReg Reward:")+ invitee.to_string()
            );
            _gstate.sing_issued += reward_sing;
        }
    }

    //  CISUM 给被邀请人（invitee）
    {
        ISSUE(
            CISUM_BANK,
            _self,
            CISUM_BONUS,
            std::string("NewReg Reward:")+ invitee.to_string()
        );
        TRANSFER(
            CISUM_BANK,
            invitee,
            CISUM_BONUS,
            std::string("NewReg Reward:")+ invitee.to_string()
        );

        // 通知（保持你原有的事件）
        notifyreward_action{
            get_self(),
            { permission_level{ get_self(), "active"_n } }
        }.send(
            CISUM_BANK,
            invitee,
            CISUM_BONUS,
            std::string("NewReg Reward:")+ invitee.to_string(),
            "signupmining"_n,
            "",
            current_time_point().time_since_epoch().count() / 1'000'000
        );
        _gstate.cisum_issued += CISUM_BONUS;
    }

    // =====  邀请人奖励：从 rewardacts 读取 act_name="invite" 的配置并发放 =====
    if (inviter.value != 0) {
        // 从 rewardacts 表中读取邀请奖励
        rewardact_t::acts_idx  acts(POE_CONTRACT, POE_CONTRACT.value);

        auto byname = acts.get_index<"byname"_n>();
        auto it = byname.find("invite"_n.value);

        CHECKC(it != byname.end(),               err::RECORD_NO_FOUND, "invite rewardact not found");
        CHECKC(it->points.symbol == CISUM_SYM,  err::SYMBOL_MISMATCH, "invite reward symbol mismatch");
        CHECKC(it->points.amount > 0,            err::NOT_POSITIVE,    "invite reward not positive");

        const asset invite_bonus = it->points;

        // 发放邀请人奖励（CISUM：mint -> transfer）
        ISSUE(
            CISUM_BANK,
            _self,
            invite_bonus,
            std::string("Invite Reward(")+ invitee.to_string()+")"
        );

        TRANSFER(
            CISUM_BANK,
            inviter,
            invite_bonus,
            std::string("Invite Reward(")+ invitee.to_string()+")"
        );

        // 发送一条 notifyreward 给邀请人
        notifyreward_action{
            get_self(),
            { permission_level{ get_self(), "active"_n } }
        }.send(
            CISUM_BANK,
            inviter,
            invite_bonus,
            std::string("Invite Reward(")+ invitee.to_string()+")",
            "invitemining",
            invitee.to_string(),
            current_time_point().time_since_epoch().count() / 1'000'000
        );

        _gstate.cisum_issued += invite_bonus;
        // 发完 invitee / inviter 奖励后，记账到 poe
        flon::poe_cisum::consumeact_action consume{
            POE_CONTRACT, { permission_level{ get_self(), "active"_n } }
        };

        consume.send(get_self(), "invite"_n, invite_bonus);
      }

}

} // namespace flon