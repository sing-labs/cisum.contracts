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

void cisumshow::publishshow(name creator,
                            const show_info& show,
                            const vector<ticket_info>& tickets)
{
  require_auth(creator);

  // 1) newshow 由 cisumshowman 自签（确保 show 合约已把 cisumshowman 加入 admin/白名单）
   NEW_SHOW(SHOW_CONTRACT,
            show.show_id,
            show.category,
            show.ticket_transferable,
            show.ticket_refundable,
            show.show_started_at,
            show.show_ended_at,
            show.show_name,
            show.show_address);

  for (const auto& ticket : tickets) {
    const nsymbol t_sym{ ticket.ticket_id };
    const nsymbol pre_sym{ ticket.prerequisite_ticket_id };
    const nasset  qty{ ticket.total_count, t_sym };

    check(ticket.price.is_valid(),      "invalid price asset");
    check(ticket.price_usdt.is_valid(),  "invalid price_usdt asset");
    check(ticket.price.amount >= 0,     "price must be >= 0");
    check(ticket.price_usdt.amount >= 0, "price_usdt must be >= 0");
    check(ticket.price_usdt.symbol.code()      == USDT_SYM.code(),      "price_usdt code must be USDT");

  if (ticket.price.amount > 0) {
      auto pcode = ticket.price.symbol.code();
      if (pcode == NESTAR_SYM.code()) {
          check(ticket.price.symbol.precision() == NESTAR_SYM.precision(), "NESTAR price precision must be 4");
      } else if (pcode == CISUM_SYM.code()) {
          check(ticket.price.symbol.precision() == CISUM_SYM.precision(), "CISUM price precision must be 8");
      }

  }

    // (A) nftcreate
    CREATE_NFT(SHOW_CONTRACT, ticket.total_count * 10,t_sym,ticket.token_uri);

    // (B) newticket（免费票这里把售卖总量记 0，库存由实收 NFT 再增）
    NEW_TICKET(SHOW_CONTRACT,
               show.show_id,
               t_sym,
               pre_sym,
               ticket.ticket_type,
               ticket.price,
               ticket.price_usdt,
               ticket.sale_started_at,
               ticket.sale_ended_at);

    // (C) 先铸到 show（保持现有流程：再由 show 转到 grab）
    ISSUE_NFT(SHOW_CONTRACT,
              creator,   // issuer
              qty,
              "issue:" + std::to_string(show.show_id) );


    // 免费票判定：金额是否为 0
    const bool is_free = (to_lower(ticket.ticket_type) == "free");
    if (is_free) {
      // 先在 grab 建 rush_sale（注意：目标合约应是 GRAB_CONTRACT）
      check(ticket.price.amount >= 0 ,"free ticket must have both price and price_usdt = 0");

      auto grab_global  = global1_singleton(GRAB_CONTRACT, GRAB_CONTRACT.value);
      auto gstate       = grab_global.get_or_default();

      // 直接读取最新的 rush_sale_id
      auto rush_sale_id = gstate.last_rush_sale_id + 1;
      ADDRUSHSALE(GRAB_CONTRACT,
                  show.show_id,
                  ticket.ticket_id,
                  ticket.sale_started_at,
                  ticket.sale_ended_at,
                  ticket.price,
                  ticket.max_grabs_per_user,
                  ticket.win_ratio);

      // 由 show 把 NFT 转给 grab，memo 带 add:<rush_sale_id>
      auto memo = "add:" + std::to_string(rush_sale_id)+":"+ std::to_string(show.show_id);

      ISSUE_TO_GRAB(SHOW_CONTRACT,
                    GRAB_CONTRACT,
                    ( nasset{ ticket.total_count, nsymbol(ticket.ticket_id) } ),
                    memo);
    }
  }
}
} // namespace flon