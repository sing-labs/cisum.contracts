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

void poh_cisum::on_transfer(const name& from,const name& to,const asset& quantity,const string& memo)
{
    if (from == get_self() || to != get_self()) return;
    CHECKC(quantity.amount > 0, err::NOT_POSITIVE, "quantity must be positive");

    // ========= 1. 解析 memo =========
    auto parts = split(memo, ":");
    CHECKC(parts.size() == 1 || parts.size() == 5, err::INVALID_FORMAT,
                "memo format must be |refuel| or |refuel:<title><amount>:<start>:<end>|");
    CHECKC(parts[0] == "refuel", err::INVALID_FORMAT, "memo must start with refuel");

    auto reward_title  = parts.size() == 1 ? "" : string(parts[1]);
    int64_t reward_raw = parts.size() == 1 ? 0 : std::stoll(string(parts[2]));
    uint32_t start_ts  = parts.size() == 1 ? 0 : std::stoul(string(parts[3]));
    uint32_t end_ts    = parts.size() == 1 ? 0 : std::stoul(string(parts[4]));

    CHECKC( reward_title.size() <= 64, err::INVALID_FORMAT, "required: reward title size <= 64" )

    if ( parts.size() == 5 ) {
        CHECKC( reward_raw > 0, err::NOT_POSITIVE, "reward amount must > 0");
        CHECKC( start_ts > current_time_point().sec_since_epoch(), err::INVALID_FORMAT, "required: start_time > now");
        CHECKC( end_ts > start_ts, err::INVALID_FORMAT, "required: end_time > start_time");
    }

    // ========= 2. 构造 reward_per_invitee =========
    int64_t mul = 1;
    for (int i = 0; i < quantity.symbol.precision(); i++) mul *= 10;
    asset reward_per_invitee(reward_raw * mul, quantity.symbol);
    if ( parts.size() == 5 ) {
        CHECKC( reward_per_invitee <= quantity, err::INSUFFICIENT_QUANTITY, "required: reward_per_invitee <= quantity" )
    }

    // ========= 3. 构造 fund_balance_s =========
    auto ext_symb = extended_symbol( quantity.symbol, get_first_receiver() );
    fund_balance_s fund_balance{
        reward_title,
        quantity,
        reward_per_invitee,
        time_point_sec(start_ts),
        time_point_sec(end_ts)
    };

    // ========= 4. 写入/合并到 inviter_fund =========
    _merge_fund_balance_s( from, ext_symb, fund_balance );
}

void poh_cisum::_merge_fund_balance_s(const name& inviter,const extended_symbol& ext_symb,const fund_balance_s& fb)
{
    inviter_fund_t::tbl_t tbl(get_self(), get_self().value);
    auto itr = tbl.find(inviter.value);

    bool is_new_inviter = (itr == tbl.end());

    // ========= 1. inviter 没有任何记录 → 必须 memo 长度为 4 =========
    if (is_new_inviter) {
        CHECKC(fb.reward_per_invitee.amount > 0, err::INVALID_FORMAT,"first refuel must specify reward_per_invitee");
        CHECKC(fb.start_time.sec_since_epoch() > 0, err::INVALID_FORMAT,"first refuel must specify start_time");
        CHECKC(fb.end_time.sec_since_epoch() > 0, err::INVALID_FORMAT,"first refuel must specify end_time");
        CHECKC(fb.start_time < fb.end_time, err::INVALID_FORMAT,"start_time must < end_time on first creation");

        tbl.emplace(get_self(), [&](auto& row){
            row.inviter = inviter;
            row.balances[ ext_symb ] = fb;
        });
        return;
    }

    tbl.modify( itr, same_payer, [&](auto& row) {

        auto it = row.balances.find(ext_symb);
        if (it == row.balances.end()) {

            CHECKC(fb.reward_per_invitee.amount > 0, err::INVALID_FORMAT,"first refuel of this symbol must specify reward_per_invitee");
            CHECKC(fb.start_time.sec_since_epoch() > 0, err::INVALID_FORMAT,"first refuel of this symbol must specify start_time");
            CHECKC(fb.end_time.sec_since_epoch() > 0, err::INVALID_FORMAT,"first refuel of this symbol must specify end_time");
            CHECKC(fb.start_time < fb.end_time, err::INVALID_FORMAT,"start_time must < end_time on first creation for this symbol");

            row.balances[ext_symb] = fb;
            return;
        }

        auto& old = it->second;

        old.available_quant += fb.available_quant;

        if (fb.reward_title != "" ) {
            old.reward_title = fb.reward_title;
        }
        if (fb.reward_per_invitee.amount > 0) {
            old.reward_per_invitee = fb.reward_per_invitee;
        }
        if (fb.start_time.sec_since_epoch() > 0) {
            old.start_time = fb.start_time;
        }
        if (fb.end_time.sec_since_epoch() > 0) {
            old.end_time = fb.end_time;
        }
    });
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
                             const name& inviter,
                             const name& invitee)
{
    // --- 基础校验 ---
    CHECKC(_gstate.registrar.value != 0,        err::RECORD_NO_FOUND, "registrar not set");
    CHECKC(submitter == _gstate.registrar,      err::DID_NOT_AUTH,    "submitter must be registrar");
    require_auth(submitter);

    CHECKC(is_account(invitee),                 err::ACCOUNT_INVALID, "invitee not exist");
    CHECKC(_gstate.platform_acct.value != 0,    err::RECORD_NO_FOUND, "platform not set");

    if (inviter.value != 0) {
        CHECKC(is_account(inviter),             err::ACCOUNT_INVALID, "inviter not exist");
        CHECKC(inviter != invitee,              err::INVALID_FORMAT,  "inviter cannot equal invitee");
    }

    // 发放 invitee 奖励
    _reward_invitee(invitee);

    // 若有 inviter
    if (inviter.value != 0) {
        _reward_inviter(inviter, invitee);

        // inviter 有专属基金 → 额外发放多币奖励
        _send_inviter_fund(inviter, invitee);
    }
}

ACTION poh_cisum::setfundinfo(
                        const name& inviter,               // 要修改的目标 inviter
                        const symbol sym,
                        const name& contract,
                        const string& reward_title,
                        const std::optional<uint32_t>& start_ts,
                        const std::optional<uint32_t>& end_ts)
{
    bool auth_self  = has_auth(get_self());
    bool auth_admin = (_gstate.registrar.value != 0 && has_auth(_gstate.registrar));
    bool auth_user  = has_auth(inviter);

    if (!(auth_self || auth_admin || auth_user)) {
        CHECKC(false, err::DID_NOT_AUTH, "no authorization to modify inviter fund");
    }

    if (!auth_self && !auth_admin) {
        CHECKC(auth_user, err::DID_NOT_AUTH, "only inviter can modify own fund");
    }

    inviter_fund_t::tbl_t tbl(get_self(), get_self().value);
    auto itr = tbl.find(inviter.value);
    CHECKC(itr != tbl.end(), err::RECORD_NO_FOUND, "inviter fund not found");

    extended_symbol es(sym, contract);

    tbl.modify(itr, get_self(), [&](auto& row){
        auto it = row.balances.find(es);
        CHECKC(it != row.balances.end(), err::RECORD_NO_FOUND,
               "symbol entry not found");

        auto& fb = it->second;

        if (reward_title != "")
            fb.reward_title = reward_title;
            
        if (start_ts.has_value()) {
            fb.start_time = time_point_sec(start_ts.value());
        }
        if (end_ts.has_value()) {
            CHECKC(end_ts.value() > fb.start_time.utc_seconds,
                   err::INVALID_FORMAT, "end_time must > start_time");
            fb.end_time = time_point_sec(end_ts.value());
        }
    });
}


void poh_cisum::_reward_invitee(const name& invitee)
{
    // ---- USDT -> SING（发给平台）----
    asset usdt = _gstate.usdt_per_user;
    CHECKC(usdt.amount > 0, err::NOT_POSITIVE, "usdt_per_user not positive");
    CHECKC(usdt.symbol == USDT_SYM, err::SYMBOL_MISMATCH, "usdt_per_user mismatch");

    asset reward_sing = exchange_asset(usdt);
    CHECKC(reward_sing.amount > 0, err::NOT_POSITIVE, "reward_sing too small");

    bool can_issue_sing =
        (_gstate.max_issued.amount > 0) &&
        (_gstate.sing_issued.amount + reward_sing.amount <= _gstate.max_issued.amount);

    if (can_issue_sing) {
        ISSUE(SING_BANK, _self, reward_sing, "NewReg Reward:" + invitee.to_string());
        TRANSFER(SING_BANK, _gstate.platform_acct, reward_sing, "NewReg Reward:" + invitee.to_string());
        _gstate.sing_issued += reward_sing;
    }

    // ---- CISUM 奖励 invitee ----
    ISSUE(CISUM_BANK, _self, CISUM_BONUS, "NewReg Reward:" + invitee.to_string());
    TRANSFER(CISUM_BANK, invitee, CISUM_BONUS, "NewReg Reward:" + invitee.to_string());

    notifyreward_action{
        get_self(),
        { permission_level{ get_self(), "active"_n } }
    }.send(
        CISUM_BANK,
        invitee,
        CISUM_BONUS,
        "NewReg Reward:" + invitee.to_string(),
        "signupmining"_n,
        "",
        current_time_point().time_since_epoch().count() / 1'000'000
    );

    _gstate.cisum_issued += CISUM_BONUS;
}

void poh_cisum::_reward_inviter(const name& inviter, const name& invitee)
{
    rewardact_t::acts_idx acts(POE_CONTRACT, POE_CONTRACT.value);
    auto byname = acts.get_index<"byname"_n>();
    auto it = byname.find("invite"_n.value);

    CHECKC(it != byname.end(), err::RECORD_NO_FOUND, "invite rewardact not found");
    CHECKC(it->points.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "invite reward mismatch");

    const asset invite_bonus = it->points;

    ISSUE(CISUM_BANK, _self, invite_bonus, "Invite Reward(" + invitee.to_string() + ")");
    TRANSFER(CISUM_BANK, inviter, invite_bonus, "Invite Reward(" + invitee.to_string() + ")");

    notifyreward_action{
        get_self(),
        { permission_level{ get_self(), "active"_n } }
    }.send(
        CISUM_BANK,
        inviter,
        invite_bonus,
        "Invite Reward(" + invitee.to_string() + ")",
        "invitemining",
        invitee.to_string(),
        current_time_point().time_since_epoch().count() / 1'000'000
    );

    _gstate.cisum_issued += invite_bonus;

    flon::poe_cisum::consumeact_action consume{
        POE_CONTRACT, { permission_level{ get_self(), "active"_n } }
    };

    consume.send(get_self(), "invite"_n, invite_bonus);
}

void poh_cisum::_send_inviter_fund(const name& inviter, const name& invitee)
{
    inviter_fund_t::tbl_t tbl(get_self(), get_self().value);
    auto itr = tbl.find(inviter.value);
    if (itr == tbl.end()) return;

    const time_point_sec now = time_point_sec(current_time_point());

    tbl.modify(itr, same_payer, [&](auto& row){

        auto& balances = row.balances;

        for (auto it = balances.begin(); it != balances.end(); ) {
            auto es  = it->first;
            auto& fb = it->second;

            if (now < fb.start_time || now > fb.end_time) {
                ++it;
                continue;
            }
            auto reward = fb.reward_per_invitee;
            if (reward.amount <= 0) {
                ++it;
                continue;
            }

            asset payout;
            bool erase_record = false;

            if (fb.available_quant.amount >= reward.amount) {
                payout = reward;
                fb.available_quant.amount -= reward.amount;

                if (fb.available_quant.amount == 0)
                    erase_record = true;

            } else {
                payout = asset(fb.available_quant.amount, fb.available_quant.symbol);
                fb.available_quant.amount = 0;
                erase_record = true;
            }

            if (payout.amount > 0) {

                name token_contract = es.get_contract();

                TRANSFER(
                    token_contract,
                    invitee,
                    payout,
                    fb.reward_title + "(" + inviter.to_string() + ")"
                );

                notifyreward_action{
                    get_self(),
                    { permission_level{ get_self(), "active"_n } }
                }.send(
                    token_contract,
                    invitee,
                    payout,
                    fb.reward_title + "(" + inviter.to_string() + ")",
                    "signupmining"_n,
                    "",
                    current_time_point().time_since_epoch().count() / 1'000'000
                );
            }

            if (erase_record) {
                it = balances.erase(it);
            } else {
                ++it;
            }
        }
    });

    auto itr2 = tbl.find(inviter.value);
    // 如果 balances 已空 → 删除整条 inviter_fund 记录
    if (itr2 != tbl.end() && itr2->balances.empty()) {
        tbl.erase(itr2);
    }
}

void poh_cisum::redeemfund(const name& oper, const name& inviter) {
    require_auth(oper);

    inviter_fund_t::tbl_t tbl(get_self(), get_self().value);
    auto itr = tbl.find(inviter.value);
    CHECKC(itr != tbl.end(), err::RECORD_NO_FOUND, "no fund record found for inviter");

    const time_point_sec now = time_point_sec(current_time_point());

    bool is_self_close = (oper == inviter);
    bool is_admin = (_gstate.registrar.value != 0 && oper == _gstate.registrar);

    // 非 inviter 本人 & 非 admin → 必须等任务结束
    if (!is_self_close && !is_admin) {
        bool can_close = true;
        for (auto& kv : itr->balances) {
            const auto& fb = kv.second;
            if (fb.end_time > now) {
                can_close = false;
                break;
            }
        }
        CHECKC(can_close, err::INVALID_FORMAT, "task not ended: end_time not reached");
    }
    // ===== 返还所有剩余 available_quant ======
    for (auto& kv : itr->balances) {
        const auto& ext_sym = kv.first;
        const auto& fb      = kv.second;

        asset refund = fb.available_quant;
        if (refund.amount <= 0) continue;

        name token_contract = ext_sym.get_contract();

        // 转给 inviter 本人
        TRANSFER(
            token_contract,
            inviter,
            refund,
            std::string("Inviter Fund Redeem: ") + inviter.to_string()
        );
    }
    tbl.erase(itr);
}

} // namespace flon