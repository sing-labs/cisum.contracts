#include "poe.cisum.hpp"

namespace flon {

using std::string;


rewardact_t poe_cisum::_get_act(const name& act_name) {
  rewardact_t::acts_idx acts(get_self(), get_self().value);
  auto byname = acts.get_index<"byname"_n>();
  auto it = byname.find(act_name.value);
  CHECKC(it != byname.end(), err::RECORD_NO_FOUND, "act not found: " + act_name.to_string());
  return *it;
}

void poe_cisum::_pay_points(const name& to, const asset& quant, const string& memo) {
  TRANSFER(CISUM_BANK, to, quant, memo)
}


void poe_cisum::awardnotice(const name&  from,
                                const name&       to,
                                const asset&      award_amount,
                                const string&     memo,
                                const name&       reward_type,
                                const string&     reward_ref_id,
                                const uint64_t&   created_at)
{
     require_auth(get_self());
}


void poe_cisum::addrewardact(const name& act_name,const asset& points,const string& memo){
  require_auth(get_self());
  CHECKC(act_name.length() > 0,       err::INVALID_FORMAT,   "act_name cannot be empty");
  CHECKC(points.symbol == CISUM_SYM, err::SYMBOL_MISMATCH,   "points symbol mismatch");
  CHECKC(points.amount >= 0,          err::INVALID_FORMAT,   "points must be non-negative");
  CHECKC(memo.size() <= 256,          err::INVALID_FORMAT,  "memo too long");

  rewardact_t::acts_idx acts(get_self(), get_self().value);
  auto byname = acts.get_index<"byname"_n>();
  auto it = byname.find(act_name.value);

  const auto now = current_time_point();

  if (it == byname.end()) {
    acts.emplace(get_self(), [&](auto& row){
      row.id              = ++_gstate.last_act_id;
      row.act_name        = act_name;
      row.points          = points;
      row.memo            = memo;
      row.claimed_points  = asset(0, CISUM_SYM);
      row.create_at       = now;
      row.update_at       = now;
    });
  } else {
    byname.modify(it, get_self(), [&](auto& row){
      row.points     = points;
      row.memo       = memo;
      row.update_at  = now;
    });
  }

  _global.set(_gstate, get_self());
}



void poe_cisum::delrewardact(const name &act_name)
{
    require_auth(get_self());

    rewardact_t::acts_idx acts(get_self(), get_self().value);
    auto byname = acts.get_index<"byname"_n>();
    auto it = byname.find(act_name.value);
    CHECKC(it != byname.end(), err::RECORD_NO_FOUND, "act not found: " + act_name.to_string());

    byname.erase(it);
}

void poe_cisum::addoperator(const name& account) {
    require_auth(get_self());

    CHECKC(account.value != 0,  err::ACCOUNT_INVALID, "operator account cannot be empty");
    CHECKC(is_account(account), err::ACCOUNT_INVALID, "operator account not exist");

    _gstate.operators.insert(account);
    _global.set(_gstate, get_self());
}

void poe_cisum::deloperator(const name&  account) {
    require_auth(get_self());

    CHECKC(account.value != 0,  err::ACCOUNT_INVALID, "operator account cannot be empty");

    auto it = _gstate.operators.find(account);
    if (it != _gstate.operators.end()) {
        _gstate.operators.erase(it);
        _global.set(_gstate, get_self());
    }
}

void poe_cisum::claimbatch(const name& submitter,
                        const name& act_name,
                        const std::vector<claim_info>& claims) {
    require_auth(submitter);
    CHECKC(_gstate.operators.count(submitter) > 0, err::DID_NOT_AUTH,
           "submitter not in operators whitelist: " + submitter.to_string());
    CHECKC(!claims.empty(), err::INVALID_FORMAT, "empty claim list");

    rewardact_t::acts_idx acts(get_self(), get_self().value);
    auto byact = acts.get_index<"byname"_n>();
    auto it = byact.find(act_name.value);
    CHECKC(it != byact.end(), err::RECORD_NO_FOUND,
           "rewardact not found by act_name: " + act_name.to_string());

    const asset base_reward = it->points;
    CHECKC(base_reward.amount > 0 && base_reward.symbol == CISUM_SYM,
           err::SYMBOL_MISMATCH, "invalid base reward points");

    int64_t total_amount = 0;
    for (const auto& c : claims) {
        CHECKC(c.claimer.value != 0 && is_account(c.claimer),
               err::ACCOUNT_INVALID, "invalid claimer: " + c.claimer.to_string());
        CHECKC(c.cnt > 0, err::INVALID_FORMAT, "invalid cnt value");

        // 检查乘法溢出：base_reward.amount * c.cnt
        CHECKC(base_reward.amount <= std::numeric_limits<int64_t>::max() / (int64_t)c.cnt,
               err::AMOUNT_TOO_LARGE, "multiplication overflow");
        int64_t reward_amt = base_reward.amount * (int64_t)c.cnt;

        // 检查加法溢出
        CHECKC(total_amount <= std::numeric_limits<int64_t>::max() - reward_amt,
               err::AMOUNT_TOO_LARGE, "addition overflow");
        total_amount += reward_amt;
    }

    CHECKC(_gstate.available_points.amount >= total_amount,
           err::INSUFFICIENT_QUANTITY, "insufficient available_points");

    _gstate.available_points.amount -= total_amount;
    _gstate.claimed_points.amount += total_amount;


    byact.modify(it, same_payer, [&](auto& row) {
        CHECKC(row.points.symbol == CISUM_SYM, err::SYMBOL_MISMATCH,
               "invalid row.points symbol");
        row.claimed_points += asset(total_amount, CISUM_SYM);
        row.update_at = current_time_point();
    });

    for (const auto& c : claims) {
        asset reward = asset((int64_t)base_reward.amount * c.cnt, CISUM_SYM);
        string memo = std::string("activereward:")+ act_name.to_string();
        CHECKC(memo.size() <= 256, err::INVALID_FORMAT, "memo too long");
        _pay_points(c.claimer, reward, memo);

        // 通知（保持你原有的事件）
        awardnotice_action{
            get_self(),
            { permission_level{ get_self(), "active"_n } }
        }.send(
            CISUM_BANK,
            c.claimer,
            reward,
            memo,
            act_name.to_string(),
            "",
            current_time_point().time_since_epoch().count() / 1'000'000
        );

    }
}

void poe_cisum::consumeact(const name& submitter, const name& act_name, const asset& amount) {

    // 允许：poe 自身 / POH 合约 / operators(submitter)
    CHECKC( has_auth(get_self())
         || has_auth(POH_CONTRACT)
         || _gstate.operators.count(submitter) > 0,
         err::DID_NOT_AUTH, " not authorized: submitter must be POE, POH, or operators");

    CHECKC(act_name.value != 0,                err::INVALID_FORMAT,   "act_name is empty");
    CHECKC(amount.is_valid(),                  err::INVALID_FORMAT,   "nvalid amount");
    CHECKC(amount.symbol == CISUM_SYM,        err::SYMBOL_MISMATCH,  "symbol mismatch");
    CHECKC(amount.amount >= 0,                 err::NOT_POSITIVE,     "amount must be non-negative");


    rewardact_t::acts_idx acts(get_self(), get_self().value);
    auto byname = acts.get_index<"byname"_n>();
    auto it = byname.find(act_name.value);
    CHECKC(it != byname.end(),                 err::RECORD_NO_FOUND,  "act not found: " + act_name.to_string());

    CHECKC(_gstate.available_points.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "available_points symbol mismatch");
    CHECKC(_gstate.claimed_points.symbol   == CISUM_SYM, err::SYMBOL_MISMATCH, "claimed_points symbol mismatch");
    CHECKC(_gstate.total_points.symbol     == CISUM_SYM, err::SYMBOL_MISMATCH, "total_points symbol mismatch");
    CHECKC(_gstate.available_points.amount >= amount.amount, err::INSUFFICIENT_QUANTITY, "insufficient available_points");

    {
        const int64_t cur = it->claimed_points.amount;
        const int64_t inc = amount.amount;
        CHECKC(cur <= std::numeric_limits<int64_t>::max() - inc,
               err::AMOUNT_TOO_LARGE, "[[11]] amount too large (act.claimed_points overflow)");
    }
    {
        const int64_t cur = _gstate.claimed_points.amount;
        const int64_t inc = amount.amount;
        CHECKC(cur <= std::numeric_limits<int64_t>::max() - inc,
               err::AMOUNT_TOO_LARGE, "[[12]] amount too large (global claimed_points overflow)");
    }

    _gstate.available_points.amount -= amount.amount;
    _gstate.claimed_points.amount   += amount.amount;

    {
        const int64_t avail   = _gstate.available_points.amount;
        const int64_t claimed = _gstate.claimed_points.amount;
        CHECKC(avail <= std::numeric_limits<int64_t>::max() - claimed,
               err::AMOUNT_TOO_LARGE, "[[13]] amount too large (total_points overflow)");
        _gstate.total_points.amount = avail + claimed;
    }

    byname.modify(it, same_payer, [&](auto& r){
        r.claimed_points.amount += amount.amount;
        r.update_at = current_time_point();
    });

}


void poe_cisum::ontransfer(const name& from, const name& to,const asset& quantity, const string& memo) {
  // 只处理打入本合约的转账
  if (from == get_self() || to != get_self()) return;

  // 基础校验
  CHECKC(get_first_receiver() == CISUM_CONTRACT, err::INVALID_FORMAT, "invalid token contract");
  CHECKC(quantity.symbol == CISUM_SYM,       err::SYMBOL_MISMATCH, "symbol mismatch");
  CHECKC(quantity.is_valid(),                 err::INVALID_FORMAT,  "invalid asset");
  CHECKC(quantity.amount > 0,                 err::NOT_POSITIVE,    "must transfer positive");

  // —— 初始化全局资产符号（首次使用时）
  if (_gstate.available_points.symbol.code().raw() == 0) {
    _gstate.available_points = asset(0, CISUM_SYM);
  } else {
    CHECKC(_gstate.available_points.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "available_points symbol mismatch");
  }
  if (_gstate.claimed_points.symbol.code().raw() == 0) {
    _gstate.claimed_points = asset(0, CISUM_SYM);
  } else {
    CHECKC(_gstate.claimed_points.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "claimed_points symbol mismatch");
  }
  if (_gstate.total_points.symbol.code().raw() == 0) {
    _gstate.total_points = asset(0, CISUM_SYM);
  } else {
    CHECKC(_gstate.total_points.symbol == CISUM_SYM, err::SYMBOL_MISMATCH, "total_points symbol mismatch");
  }

  // ========= 溢出保护 #1：available_points += quantity =========
  {
    const int64_t cur = _gstate.available_points.amount;
    const int64_t inc = quantity.amount;

    // 显式上溢检查（cur + inc <= INT64_MAX）
    CHECKC(cur <= std::numeric_limits<int64_t>::max() - inc,
           err::AMOUNT_TOO_LARGE, "amount too large (available_points overflow)");

    // 这里不需要下溢检查：inc > 0 已保证
    _gstate.available_points.amount = cur + inc;
  }

  // ========= 溢出保护 #2：total_points = available_points + claimed_points =========
  {
    const int64_t avail  = _gstate.available_points.amount;
    const int64_t claimed= _gstate.claimed_points.amount;

    // 假定 claimed_points 永远非负（按你的业务逻辑）；若需更严谨，可放开下一行：
    CHECKC(claimed >= 0, err::INVALID_FORMAT, "claimed_points negative");

    // 显式上溢检查（avail + claimed <= INT64_MAX）
    CHECKC(avail <= std::numeric_limits<int64_t>::max() - claimed,
           err::AMOUNT_TOO_LARGE, "amount too large (total_points overflow)");

    _gstate.total_points.amount = avail + claimed;
  }

  _global.set(_gstate, get_self());
}

} // namespace flon