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


mpush $show_con batchreward '["flonian",
  1761796803533,
  2101025490959,
  1,
  ["fulgwxvwfw1m","fulgwxvwfw1m","fulgwxvwfw1m","fulgwxvwfw1m"],
  "batch gift issue"
]' -p flonian

mpush show.cisum delshow '["cisum.admin","1768217670078"]' -p cisum.admin
mpush show.cisum delshow '["cisum.admin","1763368670494"]' -p cisum.admin
mpush show.cisum delshow '["cisum.admin","1764052552218"]' -p cisum.admin



mpush show.cisum setshow '["ful4oe5culzv",1766570744436,"concert",false,false,"2026-01-18T12:00:45.000","2026-01-18T12:00:45.000","1/18 VŨ THANH VÂN Solo Concert VIP Benefit",""]' -p ful4oe5culzv




setshow(const name& submitter,
                   const uint64_t&   show_id,
                   const name&       category,
                   const bool&       ticket_transferable,
                   const bool&       ticket_refundable,
                   const time_point& show_started_at,
                   const time_point& show_ended_at,
                   const string&       show_name,
                   const string&       show_address)

mpush show.cisum setshow '["ful4oe5culzv",1766569727072,"concert",false,false,"2026-01-17T12:00:44.000","2026-01-17T12:00:44.000","1/17 VŨ THANH VÂN Solo Concert VIP Benefit",""]' -p ful4oe5culzv


mpush show.cisum setshow '["ful4oe5culzv",1779207665398,"concert",false,false,"2026-05-30T13:00:00.000","2026-05-30T23:00:00.000","H3F Rumblin'\'' with Asia Tour 2026: Bandung - Tickets",""]' -p ful4oe5culzv


mpush show.cisum setshow '["ful4oe5culzv",1779208507777,"concert",false,false,"2026-05-31T10:30:00.000","2026-05-31T20:30:00.000","H3F Rumblin'\'' with Asia Tour 2026: Jakarta - Tickets",""]' -p ful4oe5culzv



mpush show.cisum  retire '[20250106,{"amount":5,"symbol":{"nid":21010001000011009}}]'  -p show.cisum