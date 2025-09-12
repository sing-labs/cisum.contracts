#include <pos.cisum/pos.cisum.hpp>
#include "safemath.hpp"
#include <utils.hpp>
#include <flon/flon.token.hpp>

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


   void pos_cisum::init(const extended_symbol& principal_token, const asset& mini_deposit_amount) {
         require_auth(get_self()); // 只允许合约自身

         check(is_account(principal_token.get_contract()),
                                 "principal_token contract not exist");
         check(principal_token.get_symbol().is_valid(),
                                 "invalid principal_token symbol");
         check(mini_deposit_amount.symbol == principal_token.get_symbol(),
                                 "mini_deposit_amount symbol must match principal_token");
         check(mini_deposit_amount.amount >= 0, "mini_deposit_amount must be >= 0");

         // 只允许初始化一次（如需可重设，改成 setglobal 动作）
         check(!_global.exists(), "already initialized");

         _gstate.admin             = get_self();          // 默认管理员 = 合约
         _gstate.principal_token   = principal_token;     // 主存款币（如 8,CISUM@cisum.token）
         _gstate.mini_deposit_amount = mini_deposit_amount;

         // 可选：如果用得到分润池/记账自增ID，给默认值
         _gstate.share_pool_id     = 0;
         _gstate.last_save_id      = 0;

   }

   void pos_cisum::withdraw(const name& issuer, const name& owner, const uint64_t& save_id) {
      require_auth( issuer );
      if ( issuer != owner ) {
         CHECKC( issuer == _gstate.admin, err::NO_AUTH, "non-admin not allowed to withdraw others saving account" )
      }

      auto save_acct = save_account_t( save_id );
      CHECKC( _db.get( owner.value, save_acct ), err::RECORD_NOT_FOUND, "account save not found" )

      auto plan = save_plan_t( save_acct.plan_id );
      CHECKC( _db.get( plan ), err::RECORD_NOT_FOUND, "plan not found: " + to_string(save_acct.plan_id) )

      // ========== ① 赎回前必须归还 MUSIC ==========
      // 缺省保护：如果老数据没有这两个字段，约定为 0@MUSIC
      asset music_reward   = save_acct.music_reward.amount   >= 0 ? save_acct.music_reward   : asset(0, MUSIC_SYMBOL);
      asset music_returned = save_acct.music_returned.amount >= 0 ? save_acct.music_returned : asset(0, MUSIC_SYMBOL);

      CHECKC(music_reward.symbol == MUSIC_SYMBOL && music_returned.symbol == MUSIC_SYMBOL,
                                                            err::SYMBOL_MISMATCH, "music reward/returned symbol mismatch");

      if (music_returned < music_reward) {
               const auto need_units = (music_reward - music_returned).amount;
               const asset need_asset{ need_units, MUSIC_SYMBOL };
               CHECKC(false, err::NO_AUTH,
                              "return MUSIC first before redeeming principal; need more: "
                              + need_asset.to_string()
                              + " (save_id=" + std::to_string(save_id) + ")");
         }
      // ② 计算可赎回本金（含提前赎回罚金）
      auto redeem_quant = save_acct.deposit_quant;
      if (plan.conf.type == deposit_type::TERM) {
         auto save_termed_at  = save_acct.created_at + plan.conf.deposit_term_days * DAY_SECONDS;
         auto now = current_time_point();
         auto premature_withdraw = (now.sec_since_epoch() < save_termed_at.sec_since_epoch());
         if (!plan.conf.allow_advance_redeem)
            CHECKC( !premature_withdraw, err::NO_AUTH, "premature withdraw not allowed" )

         if (premature_withdraw) {
            // Penalty rule (proportional):
            //   - advance_redeem_fine_rate (in BP) is the MAXIMUM penalty at t=0
            //   - actual rate scales linearly with remaining term:
            //       rate_bp = ceil(max_rate_bp * remaining_seconds / total_seconds)
            //   - penalty is applied on the *total deposited* amount
            const int64_t total_seconds    = (int64_t)plan.conf.deposit_term_days * DAY_SECONDS;
            const int64_t elapsed_seconds  = now.sec_since_epoch() - save_acct.created_at.sec_since_epoch();
            int64_t       remaining_seconds = total_seconds - elapsed_seconds;
            if (remaining_seconds < 0) remaining_seconds = 0;

            const uint64_t max_rate_bp = plan.conf.advance_redeem_fine_rate;   // e.g. 3000 = 30%
            uint64_t rate_bp = 0;
            if (total_seconds > 0 && max_rate_bp > 0) {
               // ceil(max_rate_bp * remaining / total)
               rate_bp = mul_up(max_rate_bp, (uint64_t)remaining_seconds, (uint64_t)total_seconds);
               if (rate_bp > max_rate_bp) rate_bp = max_rate_bp; // safety cap
            }

            int64_t penalty_amount = 0;
            if (rate_bp > 0) {
               penalty_amount = mul_up( save_acct.deposit_quant.amount, (int64_t)rate_bp, (int64_t)PCT_BOOST );
            }

            auto penalty = asset( penalty_amount, _gstate.principal_token.get_symbol() );

            redeem_quant -= penalty;
            CHECKC( redeem_quant.amount > 0, err::INCORRECT_AMOUNT, "redeem amount not positive " )

            // send penalty portion to penalty pool account
            TRANSFER( _gstate.principal_token.get_contract(), _gstate.penalty_share_account, penalty,
                      owner.to_string() + ":" + to_string(_gstate.share_pool_id) )
         }
      }

      // ③ 计划口径台账安全（避免负数）
      CHECKC(plan.deposit_available >= save_acct.deposit_quant,
                                                err::STATUS_ERROR, "plan.deposit_available insufficient to settle this redemption");
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

      auto now                = current_time_point();
      auto elapsed_sec        = now.sec_since_epoch() - save_acct.last_collected_at.sec_since_epoch();
      CHECKC( elapsed_sec > DAY_SECONDS, err::TIME_PREMATURE, "less than 24 hours since last interest collection time" )

      auto total_elapsed_sec  = now.sec_since_epoch() - save_acct.created_at.sec_since_epoch();

      // 用计划奖励币符号计算利息
      auto interest = asset( 0, plan.conf.interest_token.get_symbol() );
      _term_interest(save_acct.interest_rate, save_acct.deposit_quant, total_elapsed_sec, YEAR_DAYS * DAY_SECONDS, interest );

      if (interest > save_acct.interest_term_quant)
         interest = save_acct.interest_term_quant;

      auto interest_due = interest - save_acct.interest_collected;
      CHECKC( interest_due.amount > 0, err::NOT_POSITIVE, "interest due amount is zero" )

      // 可用奖励充足（>=）
      CHECKC( plan.interest_available >= interest_due, err::FEE_INSUFFICIENT, "insufficient available interest to collect" )

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
    *       1) deposit:$plan_id     -- by saver to deposit saving quantity to his or her own account
    *       2) refuel:$plan_id      -- by anyone to refuel interest pool of a plan
    *       3）return:$save_id     -- by saver to return MUSIC before redeeming principal
    */
   void pos_cisum::ontransfer(const name& from, const name& to, const asset& quant, const string& memo) {
    // 仅处理打入本合约，且禁止自转
    CHECKC(from != to, err::ACCOUNT_INVALID, "cannot transfer to self");
    if (from == get_self() || to != get_self()) return;

    // 都来自 cisum.token（本金和 MUSIC 同合约）
    const name bank = get_first_receiver();
    CHECKC(bank == _gstate.principal_token.get_contract(), err::CONTRACT_MISMATCH, "unexpected token bank");

    // ====== 分流：按符号处理 ======
    if (quant.symbol == _gstate.principal_token.get_symbol()) {
        // ------- refuel 或 deposit -------
        auto memo_params = split(memo, ":");
        bool is_refuel = (memo_params.size() == 2 && memo_params[0] == "refuel");
        bool is_deposit = (memo_params.empty() || (memo_params.size() == 2 && memo_params[0] == "deposit"));

        // ========== 【1】处理 refuel ==========
        if (is_refuel) {
            uint64_t plan_id = to_uint64(memo_params[1], "refuel plan");
            auto plan = save_plan_t(plan_id);
            CHECKC(_db.get(plan), err::RECORD_NOT_FOUND, "plan id not found: " + std::to_string(plan_id));

            // 必须匹配奖励币符号
            CHECKC(plan.conf.interest_token.get_contract() == bank, err::CONTRACT_MISMATCH,
                   "interest token contract mismatches");
            CHECKC(quant.symbol == plan.conf.interest_token.get_symbol(), err::SYMBOL_MISMATCH,
                   "interest token symbol mismatches");

            // 增加利息池余额
            plan.interest_available += quant;
            _db.set(plan);

            // 记录 refuel 日志
            _int_refuel_log(from, plan_id, quant, current_time_point());
            return;
        }

        // ========== 【2】处理 deposit ==========
        uint64_t plan_id = 1;
        if (!memo.empty()) {
            CHECKC(memo_params.size() == 2 && memo_params[0] == "deposit",
                   err::MEMO_FORMAT_ERROR,
                   "memo expects 'deposit:<plan_id>' or empty");
            plan_id = to_uint64(memo_params[1], "deposit plan");
        }

        // 取计划
        auto plan = save_plan_t(plan_id);
        CHECKC(_db.get(plan), err::RECORD_NOT_FOUND, "plan id not found: " + std::to_string(plan_id));

        // 基本金额与有效期校验
        CHECKC(quant >= _gstate.mini_deposit_amount, err::INCORRECT_AMOUNT, "deposit amount too small");
        auto now = time_point_sec(current_time_point());
        CHECKC(plan.conf.effective_from <= now, err::PLAN_INEFFECTIVE, "plan not effective yet");
        CHECKC(plan.conf.effective_to   >= now, err::PLAN_INEFFECTIVE, "plan expired already");

        // 汇总口径：本金入池
        plan.deposit_available += quant;
        _db.set(plan);

        // 创建用户存单
        auto save_acct               = save_account_t(++_gstate.last_save_id);
        save_acct.plan_id            = plan_id;
        save_acct.interest_rate      = get_interest_rate(plan.conf.ir_scheme, quant);
        save_acct.deposit_quant      = quant;
        save_acct.interest_collected = asset(0, plan.conf.interest_token.get_symbol());
        save_acct.created_at         = now;
        save_acct.term_ended_at      = now + plan.conf.deposit_term_days * DAY_SECONDS;
        save_acct.last_collected_at  = now;

        // 预计算整期可领（仅 cisumapr；nestpont 置 0）
        if (plan.conf.pool_type == "cisumapr"_n) {
            save_acct.interest_term_quant = asset(0, plan.conf.interest_token.get_symbol());
            _term_interest(
                save_acct.interest_rate,
                quant,
                (uint64_t)plan.conf.deposit_term_days * DAY_SECONDS,   // real_duration
                (uint64_t)YEAR_DAYS * DAY_SECONDS,                     // total_duration
                save_acct.interest_term_quant
            );
        } else {
            save_acct.interest_term_quant = asset(0, plan.conf.interest_token.get_symbol());
        }

        // ===== 返还 MUSIC（按计划配置的 music_reward_rate，万分制；0=不返）=====
        save_acct.music_reward   = asset(0, MUSIC_SYMBOL);
        save_acct.music_returned = asset(0, MUSIC_SYMBOL);
        if (plan.conf.music_reward_rate > 0) {
            const int64_t p_prec = get_precision(quant);
            const int64_t m_prec = get_precision(MUSIC_SYMBOL);

            __int128 base = (__int128)quant.amount;
            base = base * m_prec / p_prec;
            base = base * (__int128)plan.conf.music_reward_rate / (__int128)PCT_BOOST;

            int64_t music_amt = (int64_t)base;
            if (music_amt > 0) {
                save_acct.music_reward = asset(music_amt, MUSIC_SYMBOL);
            }
        }

        // 落存单（scope: user）
        _db.set(from.value, save_acct, false);

        // 发放 MUSIC（如需）
        if (save_acct.music_reward.amount > 0) {
            TRANSFER(MUSIC_CONTRACT, from, save_acct.music_reward,
                     std::string("pos.cisum music reward: ") + std::to_string(save_acct.save_id));
        }

        // ===== 池B：一次性发放积分（计划奖励币）=====
        if (plan.conf.pool_type == "nestpont"_n) {
            CHECKC(plan.conf.deposit_term_days > 0, err::PARAM_ERROR, "points plan requires positive term days");

            const symbol reward_sym = plan.conf.interest_token.get_symbol();
            asset points{0, reward_sym};

            const int64_t p_prec = get_precision(quant);
            const int64_t r_prec = get_precision(reward_sym);

            __int128 base = (__int128)quant.amount;
            base = base * r_prec / p_prec;
            base = base * (__int128)save_acct.interest_rate * (__int128)plan.conf.deposit_term_days;
            base /= (__int128)10000;
            base /= (__int128)365;

            points.amount = (int64_t)base;
            CHECKC(points.amount > 0, err::NOT_POSITIVE, "points = 0");

            CHECKC(plan.interest_available >= points, err::FEE_INSUFFICIENT, "insufficient interest pool for points");

            plan.interest_available -= points;
            plan.interest_redeemed  += points;
            _db.set(plan);

            TRANSFER(plan.conf.interest_token.get_contract(), from, points,
                     std::string("pos.cisum points reward: ") + std::to_string(save_acct.save_id));
        }

        return;
    }
   else if (quant.symbol == MUSIC_SYMBOL) {
      // ------- MUSIC 处理 -------
      auto ps = split(memo, ":");

      // A) 归还 return:<save_id>
      if (ps.size() == 2 && ps[0] == "return") {
         uint64_t save_id = to_uint64(ps[1], "return save_id");
         save_account_t::tbl_t accounts(get_self(), from.value);
         auto itr = accounts.find(save_id);
         check(itr != accounts.end(), "stake record not found");
         check(itr->deposit_quant.amount > 0, "stake already closed");
         check(itr->music_reward.symbol == MUSIC_SYMBOL &&
               itr->music_returned.symbol == MUSIC_SYMBOL,
               "music reward/returned symbol mismatch");
         check(itr->music_returned.amount + quant.amount <= itr->music_reward.amount,
               "exceeding music to return");

         accounts.modify(itr, same_payer, [&](auto& row) {
               row.music_returned += quant;
         });
         return;
      }

      // B) 入库 fund/refuel（充到金库）
      if (ps.size() == 1 && ps[0] == "refuel") {
         // 初始化金库
         if (_gstate.music_treasury.symbol.raw() == 0) {
               _gstate.music_treasury = asset(0, MUSIC_SYMBOL);
         }
         check(_gstate.music_treasury.symbol == MUSIC_SYMBOL,
               "music treasury symbol mismatch");

         // 增加 MUSIC 金库库存
         _gstate.music_treasury += quant;
         return;
         }
         // 其他 memo 格式一律拒绝，防止未知路径
         CHECKC(false, err::MEMO_FORMAT_ERROR,
               "invalid MUSIC memo, use 'return:<save_id>' or 'fund'/'refuel'");
   }

    // 既不是 CISUM 也不是 MUSIC
    CHECKC(false, err::SYMBOL_MISMATCH, "unsupported token symbol from cisum.token");
}



   void pos_cisum::on_nestar_transfer(const name& from, const name& to, const asset& quant, const string& memo) {
      // 只处理转账给本合约，且不处理自己给自己转账的情况
      if (to != get_self() || from == get_self()) return;

      // 校验代币符号
      check(quant.symbol == NESTAR, "only NESTAR token is accepted for refuel");

      // 解析 memo
      auto memo_params = split(memo, ":");
      check(memo_params.size() == 2 && memo_params[0] == "refuel",
            "invalid memo format, expected refuel:<plan_id>");

      // 获取计划ID
      uint64_t plan_id = to_uint64(memo_params[1], "invalid plan_id in refuel memo");

      // 加载对应 plan
      auto plan = save_plan_t(plan_id);
      CHECKC(_db.get(plan), err::RECORD_NOT_FOUND, "plan id not found: " + std::to_string(plan_id));

      // 校验 plan 奖励代币配置
      CHECKC(plan.conf.interest_token.get_contract() == get_first_receiver(),
            err::CONTRACT_MISMATCH,
            "interest token contract mismatches with plan setting");
      CHECKC(plan.conf.interest_token.get_symbol() == quant.symbol,
            err::SYMBOL_MISMATCH,
            "interest token symbol mismatches with plan setting");

      // 增加利息池余额
      plan.interest_available += quant;
      _db.set(plan);

      // 记录 refuel 日志
      _int_refuel_log(from, plan_id, quant, current_time_point());
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
      plan.conf.music_reward_rate        = pc.music_reward_rate;

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