ticket_ntoken=cvticket.nft
mreg flon $ticket_ntoken flonian
mtran flonian $ticket_ntoken "100 FLON"
mset $ticket_ntoken cvticket.nft
mcli set account permission $ticket_ntoken active --add-code


mpush $ticket_ntoken  addwhitelist '["cvticket.nft"]'   -p $ticket_ntoken

#普通票
mpush $ticket_ntoken create '[
  "'"${ticket_ntoken}"'",
  10000000000,{"value": 21010001000010016}
  ,
  "ipfs://ticket/silver6.json",
  "'"${ticket_ntoken}"'"
]' -p $ticket_ntoken


mpush $ticket_ntoken issue '[
  "'"${ticket_ntoken}"'",
  {"amount": 10000000, "symbol": {"id": 010010, "pid": 0021010001}},
  "bootstrap batch"
]' -p $ticket_ntoken


#合影票
mpush $ticket_ntoken create '[
  "'"${ticket_ntoken}"'",
  10000000000,
  {"id": 010011, "pid": 0021010001},
  "ipfs://ticket/silver1.json",
  "'"${ticket_ntoken}"'"
]' -p $ticket_ntoken


mpush $ticket_ntoken issue '[
  "'"${ticket_ntoken}"'",
  {"amount": 10000000, "symbol": {"id": 010011, "pid": 0021010001}},
  "bootstrap batch"
]' -p $ticket_ntoken



