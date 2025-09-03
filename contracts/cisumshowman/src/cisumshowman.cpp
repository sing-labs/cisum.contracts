#include "cisumshowman.hpp"
#include <flon/nasset.hpp>
#include "grab.cisum/grab.cisum.db.hpp"
#include "grab.cisum/grab.cisum.hpp"
#include "show.cisum/show.cisum.hpp"



namespace flon {

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

    // (A) nftcreate
    CREATE_NFT(SHOW_CONTRACT, ticket.total_count * 10,t_sym,ticket.token_uri);

    // (B) newticket（免费票这里把售卖总量记 0，库存由实收 NFT 再增）
    NEW_TICKET(SHOW_CONTRACT,
               show.show_id,
               t_sym,
               pre_sym,
               ticket.ticket_type,
               ticket.price,
               ticket.price_usd,
               ticket.sale_started_at,
               ticket.sale_ended_at);

    // (C) 先铸到 show（保持现有流程：再由 show 转到 grab）
    ISSUE_NFT(SHOW_CONTRACT,
              creator,   // issuer
              SHOW_CONTRACT,
              qty,
              "issue:" + std::to_string(show.show_id) );


    // 免费票判定：金额是否为 0
    const bool is_free = (ticket.price.symbol.code() == POINT_SYMBOL_CODE);
    if (is_free) {
      // 先在 grab 建 rush_sale（注意：目标合约应是 GRAB_CONTRACT）

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