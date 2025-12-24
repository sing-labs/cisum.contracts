#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

show_con=show.cisum
mreg flon $show_con flonian
mtran flonian $show_con "100 FLON"
mset $show_con show.cisum
mcli set account permission $show_con active --add-code

mpush ticket.cvnft  addwhitelist '["'"${show_con}"'"]'   -p ticket.cvnft
admin=flonian
mpush $show_con init '["'"$admin"'", "ticket.cvnft"]' -p $show_con


mpush $show_con giftbatch '["flonian",
  1761796803533,
  2101025490959,
  1,
  ["fulgwxvwfw1m","fulgwxvwfw1m","fulgwxvwfw1m","fulgwxvwfw1m"],
  "batch gift issue"
]' -p flonian

mpush show.cisum delshow '["cisum.admin","1762320833776"]' -p cisum.admin
mpush show.cisum delshow '["cisum.admin","1763368670494"]' -p cisum.admin
mpush show.cisum delshow '["cisum.admin","1764052552218"]' -p cisum.admin



mpush show.cisum setshow '["ful4oe5culzv",1766570744436,"concert",false,false,"2026-01-18T12:00:45.000","2026-01-18T12:00:45.000","1/18 VŨ THANH VÂN Solo Concert VIP Benefit",""]' -p ful4oe5culzv




mpush show.cisum setshow '["ful4oe5culzv",1766569727072,"concert",false,false,"2026-01-17T12:00:44.000","2026-01-17T12:00:44.000","1/17 VŨ THANH VÂN Solo Concert VIP Benefit",""]' -p ful4oe5culzv



setshow(const name& submitter,
                   const uint64_t&   show_id,
                   const name&       category,
                   const bool&       ticket_transferable,
                   const bool&       ticket_refundable,
                   const time_point& '2026-01-18T12:00:45.000',
                   const time_point& '2026-01-18T12:00:45.000',
                   const string&       '',
                   const string&       show_address)