#include "cisumreserve.hpp"
#include "flon/flon.token.hpp"
#include "flon/consts.hpp"

using namespace eosio;
using namespace flon;

void cisumreserve::init(const name& admin, const uint16_t& fee_bps) {
    require_auth(get_self());
    check(is_account(admin), "admin not exist");
    check(fee_bps <= 10000, "fee_bps out of range");

    _gstate.admin         = admin;
    _gstate.fee_bps       = fee_bps;
    _gstate.last_order_id = 0;
    _global.set(_gstate, get_self());
}

void cisumreserve::setfee(const name& submitter, const uint16_t& fee_bps) {
    require_auth(submitter);
    check(_gstate.admin.value != 0, "admin not set");
    check(submitter == _gstate.admin, "only admin can set fee");
    check(fee_bps <= 10000, "fee_bps out of range");
    _gstate.fee_bps = fee_bps;
    _global.set(_gstate, get_self());
}

void cisumreserve::setadmin(const name& submitter, const name& new_admin) {

    bool ok = has_auth(get_self()) || (has_auth(submitter) && submitter == _gstate.admin);
    check(ok, "requires self or current admin auth");

    check(is_account(new_admin), "new_admin not exist");
    if (new_admin == _gstate.admin) return;
    _gstate.admin = new_admin;
    _global.set(_gstate, get_self());
}

void cisumreserve::on_usdt_transfer(const name& from,
                                    const name& to,
                                    const asset& quantity,
                                    const string& memo) {
    if (from == get_self() || to != get_self()) return;

    check(from == POP_CONTRACT, "only pop can buy SING from reserve");
    check(get_first_receiver() == USDT_BANK, "only accept USDT from USDT_BANK");
    check(quantity.symbol == USDT_SYM, "only USDT accepted");
    check(quantity.amount > 0, "amount must be positive");

    // ========== 1. 先查价格 ==========
    asset price = get_price_from_swap(SING_SYM, USDT_SYM);
    check(price.amount > 0, "invalid price");

    // ========== 2. 计算兑换数量 ==========
    const int64_t p10C = pow10(SING_SYM.precision());
    __int128 num = (__int128)quantity.amount * (__int128)p10C;
    int64_t sing_units = (int64_t)(num / (__int128)price.amount);
    asset token_out{ sing_units, SING_SYM };

    // 手续费（bps）
    if (_gstate.fee_bps > 0 && token_out.amount > 0) {
        int64_t fee = (int64_t)(((__int128)token_out.amount * _gstate.fee_bps) / 10000);
        token_out.amount -= fee;
    }
    check(token_out.amount > 0, "token_out too small");

    // ========== 3. 计算汇率（每 1 USDT 可得多少 SING，放大 1e6） ==========
    uint64_t rate_ppm = (uint64_t)(
        ((__int128)pow10(SING_SYM.precision()) * 1'000'000) / (__int128)price.amount
    );

    // ========== 4. 发放 SING ==========
    flon::token::transfer_action{
        SING_BANK, { permission_level{ get_self(), "active"_n } }
    }.send(get_self(), from, token_out, "cisumreserve: buy SING");

    // ========== 5. 记录订单 ==========
    orders_idx orders(get_self(), get_self().value);
    _gstate.last_order_id += 1;
    uint64_t oid = _gstate.last_order_id;
    _global.set(_gstate, get_self());

    const auto now = current_time_point();
    orders.emplace(get_self(), [&](auto& o){
        o.id         = oid;
        o.user       = from;
        o.usdt_in    = quantity;
        o.token_out  = token_out;
        o.rate_ppm   = rate_ppm;
        o.fee_bps    = _gstate.fee_bps;
        o.created_at = now;
        o.memo       = memo;
    });
}
