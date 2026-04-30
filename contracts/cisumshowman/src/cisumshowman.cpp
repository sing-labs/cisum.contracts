#include "cisumshowman.hpp"
#include <flon/nasset.hpp>
#include "grab.cisum/grab.cisum.db.hpp"
#include "grab.cisum/grab.cisum.hpp"
#include "show.cisum/show.cisum.hpp"
#include <algorithm>
#include <string>

namespace flon {

static inline std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

// ======================================================
// 发布演出（创建 NFT 与票，但不 addupgrade）
// ======================================================
void cisumshow::publishshow(name creator,
                            const show_info& show,
                            const vector<ticket_info>& tickets)
{
    require_auth(creator);

    // 创建演出
    NEW_SHOW(SHOW_CONTRACT,
             _self,
             show.show_id,
             show.category,
             show.ticket_transferable,
             show.ticket_refundable,
             show.show_started_at,
             show.show_ended_at,
             show.show_name,
             show.show_address);

    // 获取 grab 全局 ID
    auto grab_global = global1_singleton(GRAB_CONTRACT, GRAB_CONTRACT.value);
    auto gstate      = grab_global.get_or_default();
    uint64_t next_rush_id = gstate.last_rush_sale_id;

    for (const auto& tk : tickets) {
        // const bool has_pay_ticket = (tk.pay_ticket.amount > 0 && tk.pay_ticket.symbol.is_valid());
        // if (has_pay_ticket) {
        //     continue;
        // }

        const nsymbol t_sym{ tk.ticket_id };
        const nsymbol pre_sym{ tk.prerequisite_ticket_id };
        const nasset  qty{ tk.total_count, t_sym };

        // ---- 校验价格 ----
        check(tk.price.is_valid(),        "invalid price asset");
        check(tk.price_usdt.is_valid(),   "invalid price_usdt asset");
        check(tk.price.amount  >= 0,      "price must be >= 0");
        check(tk.price_usdt.amount >= 0,  "price_usdt must be >= 0");
        check(tk.price_usdt.symbol.code() == USDT_SYM.code(), "price_usdt code must be USDT");

        // ---- 创建 NFT ----
        CREATE_NFT(SHOW_CONTRACT, _self, tk.total_count * 10, t_sym, tk.token_uri);

        if (show.ticket_transferable) {
            SET_NFT_WHITE(CVTICKET_CONTRACT, _self, tk.ticket_id, true);
        }

        // ---- 创建票档 ----
        NEW_TICKET(SHOW_CONTRACT,
                   _self,
                   show.show_id,
                   t_sym,
                   pre_sym,
                   tk.ticket_type,
                   tk.price,
                   tk.price_usdt,
                   tk.sale_started_at,
                   tk.sale_ended_at);

        // ---- 铸造 NFT ----
        ISSUE_NFT(SHOW_CONTRACT,
                  _self,
                  creator,
                  qty,
                  "issue:" + std::to_string(show.show_id));

    }
}


// ======================================================
//  激活升级票 — 手动传入 ticket_info 向 grab 注册
// ======================================================
void cisumshow::addupgrades(const name& creator,const uint64_t& show_id,const vector<ticket_info>& tickets) {
    require_auth(creator);

    // -------- 1. grab 全局状态 --------
    global1_singleton grab_global(GRAB_CONTRACT, GRAB_CONTRACT.value);
    auto gstate = grab_global.get_or_default();

    uint64_t next_rush_id = gstate.last_rush_sale_id;

    // -------- 2. 遍历票种 --------
    for (const auto& tk : tickets) {

        // -------- 2.1 activity_type 校验（票级）--------
        const bool is_rushsale    = (tk.activity_type == "rushsale"_n);
        const bool is_rushupgrade = (tk.activity_type == "rushupgrade"_n);

        check(is_rushsale || is_rushupgrade,"invalid ticket activity_type");
        check(tk.total_count > 0, "total_count must be > 0");
        // -------- 2.3 生成 rush_id --------
        next_rush_id += 1;
        const uint64_t rush_id = next_rush_id;

        // -------- 2.4 注册活动 --------
        if (is_rushsale) {
            check(tk.price.amount > 0,"rushsale requires price.amount > 0");

            ADDRUSHSALE(
                GRAB_CONTRACT,
                _self,
                show_id,
                tk.ticket_id,
                tk.sale_started_at,
                tk.sale_ended_at,
                tk.price,
                tk.max_grabs_per_user,
                tk.win_ratio);

        } else { // rushupgrade
            check(tk.pay_ticket.amount > 0 && tk.pay_ticket.symbol.is_valid(),"rushupgrade requires valid pay_ticket");
            ADDUPGRADE(
                GRAB_CONTRACT,
                _self,
                show_id,
                tk.ticket_id,
                tk.pay_ticket,
                tk.sale_started_at,
                tk.sale_ended_at,
                tk.win_ratio);
        }

        // -------- 2.5 发行 NFT 到 grab --------
        nasset issue_qty{ tk.total_count, nsymbol(tk.ticket_id) };

        std::string memo =
            (is_rushsale ? "addrushsale:" : "addrushupgrade:")
            + std::to_string(rush_id)
            + ":" + std::to_string(show_id);

        ISSUE_TO_GRAB(SHOW_CONTRACT,_self,GRAB_CONTRACT,issue_qty,memo);
    }

}

} // namespace flon
