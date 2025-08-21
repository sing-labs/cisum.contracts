show_con=show12.cisum
mreg flon $show_con flonian
mtran flonian $show_con "100 FLON"
mset $show_con show.cisum
mcli set account permission $show_con active --add-code



mpush $ticket_ntoken  addwhitelist '["'"${show_con}"'"]'   -p $ticket_ntoken

mpush $show_con init '["flonian", "cvticket.nft"]' -p $show_con

mpush $show_con addshowadm '["flonian"]' -p flonian

mpush $show_con addshowadm '["myadmin"]' -p flonian


#普通票
mpush $show_con ntcreate '[
  10000000000,
  {"id": 010013, "pid": 0021010001},
  "ipfs://ticket/silver4.json"
]' -p flonian


mpush $show_con ntissue '[
  {"amount": 10000000, "symbol": {"id": 010013, "pid": 0021010001}},
  "bootstrap batch"
]' -p flonian


#合影票
mpush $show_con ntcreate '[
  10000000000,
  {"id": 010014, "pid": 0021010001},
  "ipfs://ticket/silver5.json"
]' -p flonian


mpush $show_con ntissue '[
  {"amount": 10000000, "symbol": {"id": 010014, "pid": 0021010001}},
  "bootstrap batch"
]' -p flonian



mpush $show_con newshow '["concert", true, true, "2025-09-01T00:00:00", "2025-09-10T23:59:59", "2025-09-15T18:00:00", "2025-09-15T22:00:00", "onshelf"]' -p flonian
mpush $show_con addchecker '[1,"myadmin"]'   -p flonian

mpush $show_con newshow '["concert1", true, true, "2025-09-01T00:00:00", "2025-09-10T23:59:59", "2025-09-15T18:00:00", "2025-09-15T22:00:00", "onshelf"]' -p myadmin
mpush $show_con addchecker '[2,"myadmin"]'   -p flonian

mpush $show_con newticket '[1, {"id":010013,"pid":21010001}, {"id":0,"pid":0}, "普通票", "100.00 USDT", 100, "running"]' -p flonian


mpush $show_con issue '["gahbnbehaskk",1,21010001000010013,1,"send to gahbnbehaskk 1 ticket NFT"]' -p flonian

