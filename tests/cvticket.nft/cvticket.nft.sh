ticket_ntoken=ticket.cvnft
mreg flon $ticket_ntoken flonian
mtran flonian $ticket_ntoken "100 FLON"
mset $ticket_ntoken ticket.cvnft
mcli set account permission $ticket_ntoken active --add-code


mpush $ticket_ntoken  addwhitelist '["ticket.cvnft"]'   -p $ticket_ntoken

#普通票
mpush $ticket_ntoken create '[
  "'"${ticket_ntoken}"'",
  10000000000,{"nid": 21010001000010016}
  ,
  "ipfs://ticket/silver6.json",
  "'"${ticket_ntoken}"'"
]' -p $ticket_ntoken


mpush $ticket_ntoken issue '[
  "'"${ticket_ntoken}"'",
  {"amount": 10000000, "symbol": {"nid": 21010001000010010}},
  "bootstrap batch"
]' -p $ticket_ntoken


#合影票
mpush $ticket_ntoken create '[
  "'"${ticket_ntoken}"'",
  10000000000,
  {"nid": 21010001000010011},
  "ipfs://ticket/silver1.json",
  "'"${ticket_ntoken}"'"
]' -p $ticket_ntoken


mpush $ticket_ntoken issue '[
  "'"${ticket_ntoken}"'",
  {"amount": 10000000, "symbol": {"nid": 21010001000010011}},
  "bootstrap batch"
]' -p $ticket_ntoken










ticket_ntoken=ticket.cvnft
mpush $ticket_ntoken create '[
  "'"${ticket_ntoken}"'",
  10000000000,{"nid": 21010001000010016}
  ,
  "ipfs://ticket/silver6.json",
  "'"${ticket_ntoken}"'"
]' -p $ticket_ntoken


mpush $ticket_ntoken issue '[
  "'"${ticket_ntoken}"'",
  {"amount": 10000000, "symbol": {"nid": 21010001000010010}},
  "bootstrap batch"
]' -p $ticket_ntoken
