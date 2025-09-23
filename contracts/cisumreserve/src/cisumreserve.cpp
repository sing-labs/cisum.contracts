#include "cisumreserve.hpp"
#include "flon/flon.token.hpp"

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

    check(get_first_receiver() == USDT_BANK, "only accept USDT from USDT_BANK");
    check(quantity.symbol == USDT_SYM, "only USDT accepted");
    check(quantity.amount > 0, "amount must be positive");

    // 价格换算：USDT -> CISUM（按 flon.swap 价格）
    asset cisum_out = usdt_to_cisum(quantity);

    // 手续费（bps）
    if (_gstate.fee_bps > 0 && cisum_out.amount > 0) {
        int64_t fee = (int64_t)(((__int128)cisum_out.amount * _gstate.fee_bps) / 10000);
        cisum_out.amount -= fee;
    }
    check(cisum_out.amount > 0, "cisum_out too small");

    // 记录订单汇率：每 1 USDT 可得多少 CISUM（放大 1e6）
    // price.amount = USDT(最小单位) / 1 CISUM
    asset price = get_price_from_swap_as_asset(CISUM_SYM, USDT_SYM);
    check(price.amount > 0, "invalid price");
    uint64_t rate_ppm = (uint64_t)(
        ((__int128)pow10(CISUM_SYM.precision()) * 1'000'000) / (__int128)price.amount
    );

    // 向用户发放 CISUM
    flon::token::transfer_action{
        CISUM_BANK, { permission_level{ get_self(), "active"_n } }
    }.send(get_self(), from, cisum_out, "cisumreserve: buy SING");

    // 记录订单
    orders_idx orders(get_self(), get_self().value);
    _gstate.last_order_id += 1;
    uint64_t oid = _gstate.last_order_id;
    _global.set(_gstate, get_self());
    const auto now = current_time_point();
    orders.emplace(get_self(), [&](auto& o){
        o.id         = oid;
        o.user       = from;
        o.usdt_in    = quantity;
        o.cisum_out  = cisum_out;
        o.rate_ppm   = rate_ppm;
        o.fee_bps    = _gstate.fee_bps;
        o.created_at = now;
        o.memo       = memo;
    });
}


void cisumreserve::on_cisum_transfer(const name& from,
                                     const name& to,
                                     const asset& quantity,
                                     const std::string& memo) {
    if (from == get_self() || to != get_self()) return;

    check(quantity.symbol == CISUM_SYM, "only SING deposits allowed");
    check(quantity.amount > 0, "amount must be positive");

}