show_con=show14.cisum
mreg flon $show_con flonian
mtran flonian $show_con "100 FLON"
mset $show_con show.cisum
mcli set account permission $show_con active --add-code



mpush $ticket_ntoken  addwhitelist '["'"${show_con}"'"]'   -p $ticket_ntoken

mpush $show_con init '["flonian", "cvticket.nft"]' -p $show_con

mpush $show_con addshowadm '["flonian"]' -p flonian

mpush $show_con addshowadm '["myadmin"]' -p flonian


#普通票
mpush $show_con nftcreate '[
  10000000000,{"value": 21010001000010019},
  "ipfs://ticket/silver9.json"
]' -p flonian


mpush $show_con nftissue '[
  {"amount": 10000000, "symbol": {"value": 21010001000010019} },
  "bootstrap batch"
]' -p flonian


#合影票
mpush $show_con nftcreate '[
  10000000000,{"value": 21010001000010020},
  "ipfs://ticket/silver10.json"
]' -p flonian


mpush $show_con nftissue '[
  {"amount": 10000000, "symbol": {"value": 21010001000010020} },
  "bootstrap batch"
]' -p flonian



mpush $show_con newshow '[41256,"concert", true, true, "2025-09-01T00:00:00", "2025-09-10T23:59:59","test" ,"gasgsgg gq gg sa gdf jhfd j" , "onshelf"]' -p flonian
mpush $show_con addchecker '[41256,"myadmin"]'   -p flonian

mpush $show_con newshow '[41257,"concert", true, true, "2025-09-01T00:00:00", "2025-09-10T23:59:59","test" ,"gasgsgg gq gg sa gdf jhfd j" , "onshelf"]' -p flonian
mpush $show_con addchecker '[41257,"myadmin"]'   -p flonian

mpush $show_con newticket '[41256, {"value": 21010001000010019}, {"value": 0}, "普通票", "100.00 USDT", 100, "running", "2025-09-01T00:00:00", "2025-09-10T23:59:59"]' -p flonian


mpush $show_con issue '["gahbnbehaskk",41256,21010001000010019,1,"send to gahbnbehaskk 1 ticket NFT"]' -p flonian

