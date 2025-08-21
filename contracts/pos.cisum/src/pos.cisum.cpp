#include <pos.cisum/pos.cisum.hpp>
#include "safemath.hpp"
#include <utils.hpp>


static constexpr eosio::name active_perm        {"active"_n};

#define TRANSFER(bank, to, quantity, memo) \
    {	eosio::token::transfer_action act{ bank, { {_self, active_perm} } };\
			act.send( _self, to, quantity , memo );}

namespace flon {

using namespace std;
using namespace wasm::safemath;

#define CHECKC(exp, code, msg) \
   { if (!(exp)) eosio::check(false, string("[[") + to_string((int)code) + string("]] ") + msg); }

   inline int64_t get_precision(const symbol &s) {
      int64_t digit = s.precision();
      CHECK(digit >= 0 && digit <= 18, "precision digit " + std::to_string(digit) + " should be in range[0,18]");
      return calc_precision(digit);
   }

   inline int64_t get_precision(const asset &a) {
      return get_precision(a.symbol);
   }

   //lad3: 12 mon, 365 days
   inline uint64_t get_ir_ladder12m( const asset& quant ) {
      if( quant.amount <= (1000 * get_precision(quant) ))   return 800;    // 8%
      if( quant.amount <= (2000 * get_precision(quant) ))   return 1000;   // 10%
                                                            return 1200;   // 12%
   }

   //lad2: 6 mon, 180 days
   inline uint64_t get_ir_ladder6m( const asset& quant ) {
      if( quant.amount <= (1000 * get_precision(quant) ))   return 680;    // 6.8%
      if( quant.amount <= (2000 * get_precision(quant) ))   return 800;    // 8%
                                                            return 1000;   // 10%
   }

   //lad1: 3 mon, 90 days
   inline uint64_t get_ir_ladder3m( const asset& quant ) {
      if( quant.amount <= (1000 * get_precision(quant) ))   return 600;    // 6%
      if( quant.amount <= (2000 * get_precision(quant) ))   return 680;    // 6.8%
                                                            return 800;    // 8%
   }

   inline uint64_t get_ir_dm1() {
      return 100; // 1%
   }
   inline uint64_t get_ir_dm2() {
      return 200; // 2%
   }
   inline uint64_t get_ir_dm3() {
      return 300; // 3%
   }

   inline uint64_t get_interest_rate( const name& ir_scheme, const asset& quant ) {
      switch( ir_scheme.value ) {
         case interest_rate_scheme::LADDER1.value : return get_ir_ladder3m(quant);
         case interest_rate_scheme::LADDER2.value : return get_ir_ladder6m(quant);
         case interest_rate_scheme::LADDER3.value : return get_ir_ladder12m(quant);

         case interest_rate_scheme::LADDER11.value : return 600;
         case interest_rate_scheme::LADDER21.value : return 630;
         case interest_rate_scheme::LADDER31.value : return 660;

         case interest_rate_scheme::DEMAND1.value : return get_ir_dm1();
         case interest_rate_scheme::DEMAND2.value : return get_ir_dm2();
         case interest_rate_scheme::DEMAND3.value : return get_ir_dm3();
         default:                                   return get_ir_dm1();
      }
   }

   inline void _term_interest( const uint64_t interest_rate, const asset& deposit_quant,
                              const uint64_t real_duration, const uint64_t& total_duraton, asset& interest ) {
      CHECKC ( real_duration > 0, err::PARAM_ERROR, "invald param ")
      interest.amount = mul_down( mul_down(interest_rate * 100, real_duration, total_duraton), deposit_quant.amount, PCT_BOOST * 100 );
   }


   void pos_cisum::init() {
      CHECK(false, "disabled" )
      require_auth( _self );

   }

   void pos_cisum::withdraw(const name& issuer, const name& owner, const uint64_t& save_id) {
      require_auth( issuer );
      // check(false, "under maintenance");

      if ( issuer != owner ) {
         CHECKC( issuer == _gstate.admin, err::NO_AUTH, "non-admin not allowed to withdraw others saving account" )
      }

      auto save_acct = save_account_t( save_id );
      CHECKC( _db.get( owner.value, save_acct ), err::RECORD_NOT_FOUND, "account save not found" )

      auto plan = save_plan_t( save_acct.plan_id );
      CHECKC( _db.get( plan ), err::RECORD_NOT_FOUND, "plan not found: " + to_string(save_acct.plan_id) )

      auto redeem_quant             = save_acct.deposit_quant;
      if (plan.conf.type == deposit_type::TERM) {
         auto save_termed_at        = save_acct.created_at + plan.conf.deposit_term_days * DAY_SECONDS;
         auto now = current_time_point();
         auto premature_withdraw = (now.sec_since_epoch() < save_termed_at.sec_since_epoch());
         if (!plan.conf.allow_advance_redeem)
            CHECKC( !premature_withdraw, err::NO_AUTH, "premature withdraw not allowed" )

         if (premature_withdraw) {
            auto unfinish_rate      = div( save_termed_at.sec_since_epoch() - now.sec_since_epoch(), plan.conf.deposit_term_days * DAY_SECONDS, PCT_BOOST );
            auto penalty_amount     = mul_up( mul_up( save_acct.deposit_quant.amount, unfinish_rate, PCT_BOOST ), plan.conf.advance_redeem_fine_rate, PCT_BOOST );
            auto penalty            = asset( penalty_amount, _gstate.principal_token.get_symbol() );
            redeem_quant            -= penalty;
            CHECKC( redeem_quant.amount > 0, err::INCORRECT_AMOUNT, "redeem amount not positive " )

            TRANSFER( _gstate.principal_token.get_contract(), _gstate.penalty_share_account, penalty, owner.to_string() + ":" + to_string(_gstate.share_pool_id) )
         }
      }

      plan.deposit_available        -= save_acct.deposit_quant;
      plan.deposit_redeemed         += redeem_quant;
      _db.set( plan );
      _db.del( owner.value, save_acct );

      TRANSFER( _gstate.principal_token.get_contract(), owner, redeem_quant, "redeem: " + to_string(save_id) )
   }

   void pos_cisum::collectint(const name& issuer, const name& owner, const uint64_t& save_id) {
      require_auth( issuer );
      if ( issuer != owner ) {
         CHECKC( issuer == _gstate.admin, err::NO_AUTH, "non-admin not allowed to collect others saving interest" )
      }

      auto save_acct = save_account_t( save_id );
      CHECKC( _db.get( owner.value, save_acct ), err::RECORD_NOT_FOUND, "account save not found" )

      auto plan = save_plan_t( save_acct.plan_id );
      CHECKC( _db.get( plan ), err::RECORD_NOT_FOUND, "plan not found: " + to_string(save_acct.plan_id) )

      // 仅池A可领取
      CHECKC( plan.conf.pool_type == "cisumapr"_n, err::STATUS_ERROR, "collectint only for cisumapr pool" )

      if (save_acct.last_collected_at == time_point())
         save_acct.last_collected_at = save_acct.created_at;

      auto now               = current_time_point();
      auto elapsed_sec       = now.sec_since_epoch() - save_acct.last_collected_at.sec_since_epoch();
      CHECKC( elapsed_sec > DAY_SECONDS, err::TIME_PREMATURE, "less than 24 hours since last interest collection time" )
      auto total_elapsed_sec = now.sec_since_epoch() - save_acct.created_at.sec_since_epoch();

      // 用计划奖励币符号
      auto interest = asset( 0, plan.conf.interest_token.get_symbol() );
      _term_interest(save_acct.interest_rate, save_acct.deposit_quant, total_elapsed_sec, YEAR_DAYS * DAY_SECONDS, interest );
      if (interest > save_acct.interest_term_quant)
         interest = save_acct.interest_term_quant;

      auto interest_due = interest - save_acct.interest_collected;
      CHECKC( interest_due.amount > 0, err::NOT_POSITIVE, "interest due amount is zero" )

      // 可用奖励充足（>=）
      CHECKC( plan.interest_available >= interest_due, err::NOT_POSITIVE, "insufficient available interest to collect" )

      // 发放奖励：使用计划奖励币合约
      TRANSFER( plan.conf.interest_token.get_contract(), owner, interest_due, "interest: " + to_string(save_id) )

      save_acct.interest_collected += interest_due;
      save_acct.last_collected_at  = now;
      _db.set( owner.value, save_acct );

      plan.interest_available -= interest_due;
      plan.interest_redeemed  += interest_due;
      _db.set( plan );

      _int_coll_log(owner, save_acct.save_id, plan.id, interest_due, time_point_sec(current_time_point()));
   }


   /**
    * @brief send nasset tokens into nftone marketplace
    *
    * @param from
    * @param to
    * @param quantity
    * @param memo: two formats:
    *       0) <NULL>               -- by saver to deposit to plan_id=1
    *       1) refuel:$plan_id      -- by admin to deposit interest quantity
    *       2) deposit:$plan_id     -- by saver to deposit saving quantity to his or her own account
    *
    */
   void pos_cisum::ontransfer(const name& from, const name& to, const asset& quant, const string& memo) {
      CHECKC( from != to, err::ACCOUNT_INVALID, "cannot transfer to self" );
      if (from == get_self() || to != get_self()) return;

      const name token_bank = get_first_receiver();
      vector<string_view> memo_params = split(memo, ":");

      // 解析 plan_id
      uint64_t plan_id = 1;
      bool is_refuel   = (memo_params.size() == 2 && memo_params[0] == "refuel");
      bool is_deposit  = (memo_params.size() == 2 && memo_params[0] == "deposit");
      if (is_refuel || is_deposit)
         plan_id = to_uint64(memo_params[1], is_refuel ? "refuel plan" : "deposit plan");

      auto plan = save_plan_t( plan_id );
      CHECKC( _db.get( plan ), err::RECORD_NOT_FOUND, "plan id not found: " + to_string( plan_id ) )

      // refuel：必须是计划奖励币
      if (is_refuel) {
         CHECKC( plan.conf.interest_token.get_contract() == token_bank, err::CONTRACT_MISMATCH, "interest token contract mismatches" )
         CHECKC( quant.symbol == plan.conf.interest_token.get_symbol(), err::SYMBOL_MISMATCH, "interest token symbol mismatches" )

         plan.interest_available += quant;
         _db.set( plan );
         _int_refuel_log(from, plan_id, quant, current_time_point());
         return;
      }

      // deposit：必须是全局本金
      CHECKC( _gstate.mini_deposit_amount <= quant, err::INCORRECT_AMOUNT, "deposit amount too small" )
      CHECKC( _gstate.principal_token.get_contract() == token_bank, err::CONTRACT_MISMATCH, "deposit token contract mismatches" )
      CHECKC( quant.symbol == _gstate.principal_token.get_symbol(), err::SYMBOL_MISMATCH, "deposit token symbol mismatches" )

      auto now = time_point_sec(current_time_point());
      CHECKC( plan.conf.effective_from <= now, err::PLAN_INEFFECTIVE, "plan not effective yet" )
      CHECKC( plan.conf.effective_to   >= now, err::PLAN_INEFFECTIVE, "plan expired already" )

      plan.deposit_available += quant;
      _db.set( plan );

      auto save_acct               = save_account_t( ++_gstate.last_save_id );
      save_acct.plan_id            = plan_id;
      save_acct.interest_rate      = get_interest_rate( plan.conf.ir_scheme, quant );
      save_acct.deposit_quant      = quant; // 本金
      save_acct.interest_collected = asset(0, plan.conf.interest_token.get_symbol());
      save_acct.created_at         = now;
      save_acct.term_ended_at      = now + plan.conf.deposit_term_days * DAY_SECONDS;
      save_acct.last_collected_at  = now;

      if (plan.conf.pool_type == "cisumapr"_n) {
         // 池A：预计算整期可领
         save_acct.interest_term_quant = asset(0, plan.conf.interest_token.get_symbol());
         _term_interest(save_acct.interest_rate, quant, plan.conf.deposit_term_days, YEAR_DAYS, save_acct.interest_term_quant );
      } else {
         // 池B：term_quant 不使用，置0
         save_acct.interest_term_quant = asset(0, plan.conf.interest_token.get_symbol());
      }

      _db.set( from.value, save_acct, false );

      // 池B：一次性发放积分（计划奖励币）
      if (plan.conf.pool_type == "nestpont"_n) {
         CHECKC( plan.conf.deposit_term_days > 0, err::PARAM_ERROR, "points plan requires positive term days" );
         asset points{0, plan.conf.interest_token.get_symbol()};
         {
            __int128 raw = (__int128)quant.amount * (__int128)save_acct.interest_rate * (__int128)plan.conf.deposit_term_days;
            raw /= (__int128)10000;  // BP
            raw /= (__int128)365;
            points.amount = (int64_t)raw;
         }
         CHECKC( points.amount > 0, err::NOT_POSITIVE, "points = 0" );
         TRANSFER( plan.conf.interest_token.get_contract(), from, points, "pos.cisum points reward: " + to_string(save_acct.save_id) );
      }
   }

void pos_cisum::setplan(const uint64_t& pid, const plan_conf_s& pc) {
   require_auth( _gstate.admin );
   CHECKC( pc.type == deposit_type::TERM || pc.type == deposit_type::DEMAND,
           err::PARAM_ERROR, "invalid deposit type" );
   CHECKC( pc.pool_type == "cisumapr"_n || pc.pool_type == "nestpont"_n,
           err::PARAM_ERROR, "invalid pool_type (expect 'cisumapr' or 'nestpont')" );

   // TERM 要求正的期限；DEMAND 可为 0（随存随取）
   if (pc.type == deposit_type::TERM) {
      CHECKC( pc.deposit_term_days > 0, err::PARAM_ERROR, "term deposit requires positive deposit_term_days" );
   }

   CHECKC( pc.effective_from <= pc.effective_to, err::PARAM_ERROR, "invalid effective time range" );

   CHECKC( pc.interest_token.get_contract().value != 0, err::PARAM_ERROR, "interest_token contract not set" );
   CHECKC( pc.interest_token.get_symbol().is_valid(), err::PARAM_ERROR, "invalid interest_token symbol" );

   auto plan = save_plan_t(pid);
   bool plan_existing = _db.get( plan );
   plan.conf.type                     = pc.type;
   plan.conf.pool_type                = pc.pool_type;
   plan.conf.ir_scheme                = pc.ir_scheme;
   plan.conf.deposit_term_days        = pc.deposit_term_days;
   plan.conf.allow_advance_redeem     = pc.allow_advance_redeem;
   plan.conf.advance_redeem_fine_rate = pc.advance_redeem_fine_rate;
   plan.conf.effective_from           = pc.effective_from;
   plan.conf.effective_to             = pc.effective_to;
   plan.conf.interest_token           = pc.interest_token;

   if (!plan_existing) {
      const auto zero_principal = asset(0, _gstate.principal_token.get_symbol());
      const auto zero_interest  = asset(0, pc.interest_token.get_symbol());

      plan.deposit_available  = zero_principal;
      plan.deposit_redeemed   = zero_principal;
      plan.interest_available = zero_interest;
      plan.interest_redeemed  = zero_interest;
      plan.created_at         = current_time_point();

   } else {
      // 已存在计划：

      // 1) 本金口径：必须与全局本金一致；不允许更换
      CHECKC( plan.deposit_available.symbol == _gstate.principal_token.get_symbol(),
              err::SYMBOL_MISMATCH, "principal symbol mismatch with global setting (deposit_available)" );
      CHECKC( plan.deposit_redeemed.symbol  == _gstate.principal_token.get_symbol(),
              err::SYMBOL_MISMATCH, "principal symbol mismatch with global setting (deposit_redeemed)" );

      // 2) 奖励口径：若存在余额，禁止更换奖励符号；若两边均为 0，可切换到新符号
      const bool interest_has_balance =
         (plan.interest_available.amount != 0) || (plan.interest_redeemed.amount != 0);

      if (interest_has_balance) {
         CHECKC( plan.interest_available.symbol == pc.interest_token.get_symbol(),
                 err::SYMBOL_MISMATCH, "cannot change interest token symbol when non-zero interest balances exist" );
         // 保持原符号，无需改动资产字段
      } else {
         // 无余额：更新奖励资产符号为最新的计划奖励币
         plan.interest_available = asset(0, pc.interest_token.get_symbol());
         plan.interest_redeemed  = asset(0, pc.interest_token.get_symbol());
      }
   }

   _db.set( plan );
}

   void pos_cisum::delplan(const uint64_t& pid) {
      require_auth( _gstate.admin );

      auto plan = save_plan_t(pid);
      CHECKC(_db.get( plan ), err::RECORD_NOT_FOUND, "plan not exist: " + to_string(pid) )

      _db.del( plan );
   }

   void pos_cisum::_int_coll_log(const name& account, const uint64_t& account_id, const uint64_t& plan_id, const asset &quantity, const time_point& created_at) {
      pos_cisum::interest_withdraw_log_action act{ _self, { {_self, active_perm} } };
      act.send( account, account_id, plan_id, quantity, created_at );
   }

   void pos_cisum::_int_refuel_log(const name& refueler, const uint64_t& plan_id, const asset &quantity, const time_point& created_at) {
      pos_cisum::intrefuellog_action act{ _self, { {_self, active_perm} } };
      act.send( refueler, plan_id, quantity, created_at );
   }

   void pos_cisum::intrefuellog(const name& refueler, const uint64_t& plan_id, const asset &quantity, const time_point& created_at) {
      require_auth(get_self());
      require_recipient(refueler);
   }

   void pos_cisum::intcolllog(const name& account, const uint64_t& account_id, const uint64_t& plan_id, const asset &quantity, const time_point& created_at) {
      require_auth(get_self());
      require_recipient(account);
   }

} //namespace flon