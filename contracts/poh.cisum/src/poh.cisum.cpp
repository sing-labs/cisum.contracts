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
    CHECKC(parts.size() == 4, err::INVALID_FORMAT,
           "memo format must be reward:<amount>:<start>:<end>");
    CHECKC(parts[0] == "reward", err::INVALID_FORMAT, "memo must start with reward");

    int64_t reward_raw = std::stoll(string(parts[1]));
    uint32_t start_ts  = std::stoul(string(parts[2]));
    uint32_t end_ts    = std::stoul(string(parts[3]));

    CHECKC(reward_raw > 0, err::NOT_POSITIVE, "reward amount must > 0");
    CHECKC(end_ts > start_ts, err::INVALID_FORMAT, "end_time must > start_time");

    // ========= 2. 构造 reward_per_invitee =========
    int64_t mul = 1;
    for (int i = 0; i < quantity.symbol.precision(); i++) mul *= 10;

    asset reward_per_invitee(reward_raw * mul, quantity.symbol);

    // ========= 3. 构造 token_balance =========
    token_balance tb{
        extended_asset(quantity, get_first_receiver()),
        reward_per_invitee,
        time_point_sec(start_ts),
        time_point_sec(end_ts)
    };

    // ========= 4. 写入/合并到 inviter_fund =========
    name inviter = from;
    _merge_token_balance(inviter, tb);
}

void poh_cisum::_merge_token_balance(const name& inviter, const token_balance& tb)
{
    inviter_fund_t::tbl_t tbl(get_self(), get_self().value);
    auto itr = tbl.find(inviter.value);

    if (itr == tbl.end()) {
        // 👉 首次创建
        tbl.emplace(get_self(), [&](auto& row){
            row.inviter_account = inviter;
            row.balances.push_back(tb);
        });
        return;
    }

    // 👉 已存在 → merge 逻辑
    tbl.modify(itr, same_payer, [&](auto& row){
        for (auto& b : row.balances) {
            // 按币种 + 合约判断是否同一类型奖励
            if (b.available_quant.quantity.symbol == tb.available_quant.quantity.symbol &&
                b.available_quant.contract == tb.available_quant.contract)
            {
                b.available_quant.quantity       += tb.available_quant.quantity;
                b.reward_quant_per_invitee        = tb.reward_quant_per_invitee;
                b.start_time                      = tb.start_time;
                b.end_time                        = tb.end_time;
                return;
            }
        }

        // 没找到 → 插入新的币种记录
        row.balances.push_back(tb);
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
    if (inviter.value != 0)
    {
        _reward_inviter(inviter, invitee);

        // === inviter_fund_t 检查 ===
        inviter_fund_t::tbl_t tbl(_self, _self.value);
        if (tbl.find(inviter.value) != tbl.end()) {
            // inviter 有专属基金 → 额外发放多币奖励
            _payout_inviter_fund(inviter, invitee);
        }
    }
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
void poh_cisum::_payout_inviter_fund(const name& inviter, const name& invitee)
{
    inviter_fund_t::tbl_t tbl(get_self(), get_self().value);
    auto itr = tbl.find(inviter.value);
    if (itr == tbl.end()) return;   // 无奖励记录，直接跳过

    time_point_sec now = time_point_sec(current_time_point());

    tbl.modify(itr, same_payer, [&](auto& row){

        for (auto& tb : row.balances)
        {
            if (now < tb.start_time || now > tb.end_time) {
                continue;
            }

            asset reward = tb.reward_quant_per_invitee;
            if (reward.amount <= 0) continue;

            CHECKC(tb.available_quant.quantity.amount >= reward.amount,
                err::QUANTITY_INSUFFICIENT,
               "insufficient inviter_fund balance for symbol: "
                + reward.symbol.code().to_string()
            );

            tb.available_quant.quantity -= reward;

            name token_contract = tb.available_quant.contract;

            TRANSFER(
                token_contract,
                invitee,
                reward,
                "NewReg Reward:" + invitee.to_string()
            );

            notifyreward_action{
                get_self(),
                { permission_level{ get_self(), "active"_n } }
            }.send(
                token_contract,
                invitee,
                reward,
                "NewReg Reward:"+ invitee.to_string(),
                "signupmining"_n,
                "",
                current_time_point().time_since_epoch().count() / 1'000'000
            );
        }
    });
}


} // namespace flon