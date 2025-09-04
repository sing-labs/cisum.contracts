#include "poe.cisum.hpp"
#include "flon.token.hpp"

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
  TRANSFER(POINTS_BANK, to, quant, memo)
}

void poe_cisum::addrewardact(name act_name, asset points, string memo) {
  require_auth(get_self());
  CHECKC(act_name.length() > 0,       err::INVALID_FORMAT,   "act_name cannot be empty");
  CHECKC(points.symbol == POINTS_SYM, err::SYMBOL_MISMATCH,  "points symbol mismatch");
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
      row.claimed_points  = asset(0, POINTS_SYM);
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


void poe_cisum::delrewardact(name act_name) {
  require_auth(get_self());

  rewardact_t::acts_idx acts(get_self(), get_self().value);
  auto byname = acts.get_index<"byname"_n>();
  auto it = byname.find(act_name.value);
  CHECKC(it != byname.end(), err::RECORD_NO_FOUND, "act not found: " + act_name.to_string());

  byname.erase(it);
}

void poe_cisum::claimpoints(name claimer, name act_name) {
  require_auth(claimer);

  rewardact_t::acts_idx acts(get_self(), get_self().value);
  auto byname = acts.get_index<"byname"_n>();
  auto it = byname.find(act_name.value);
  CHECKC(it != byname.end(), err::RECORD_NO_FOUND, "act not found: " + act_name.to_string());

  const asset reward = it->points;
  CHECKC(reward.amount > 0, err::NOT_POSITIVE, "act points is zero");
  CHECKC(reward.symbol == POINTS_SYM, err::SYMBOL_MISMATCH, "act points symbol mismatch");

  CHECKC(_gstate.available_points.symbol == POINTS_SYM, err::SYMBOL_MISMATCH, "global available_points symbol mismatch");
  CHECKC(_gstate.claimed_points.symbol   == POINTS_SYM, err::SYMBOL_MISMATCH, "global claimed_points symbol mismatch");
  CHECKC(_gstate.total_points.symbol     == POINTS_SYM, err::SYMBOL_MISMATCH, "global total_points symbol mismatch");
  CHECKC(_gstate.available_points.amount >= reward.amount, err::INSUFFICIENT_QUANTITY, "insufficient available_points");

  _gstate.available_points -= reward;
  _gstate.claimed_points   += reward;
  _gstate.total_points      = _gstate.available_points + _gstate.claimed_points;

  byname.modify(it, get_self(), [&](auto& row){
    row.claimed_points += reward;
    row.update_at       = current_time_point();
  });

  _global.set(_gstate, get_self());

  _pay_points(claimer, reward, string("poe reward: ") + act_name.to_string());
}

void poe_cisum::ontransfer(name from, name to, asset quantity, string memo) {
  if (from == get_self() || to != get_self()) return; // 只处理转入合约
  CHECKC(get_first_receiver() == POINTS_BANK, err::INVALID_FORMAT, "invalid token contract");
  CHECKC(quantity.symbol == POINTS_SYM,       err::SYMBOL_MISMATCH, "symbol mismatch");
  CHECKC(quantity.amount > 0,                 err::NOT_POSITIVE,    "must transfer positive");

  if (_gstate.available_points.symbol.code().raw() == 0) {
    _gstate.available_points = asset(0, POINTS_SYM);
  }
  if (_gstate.claimed_points.symbol.code().raw() == 0) {
    _gstate.claimed_points = asset(0, POINTS_SYM);
  }
  if (_gstate.total_points.symbol.code().raw() == 0) {
    _gstate.total_points = asset(0, POINTS_SYM);
  }

  _gstate.available_points += quantity;
  _gstate.total_points      = _gstate.available_points + _gstate.claimed_points;
  _global.set(_gstate, get_self());
}

} // namespace flon