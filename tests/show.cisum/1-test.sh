show_con=show23.cisum
mreg flon $show_con flonian
mtran flonian $show_con "100 FLON"
mset $show_con show.cisum
mcli set account permission $show_con active --add-code



mpush $ticket_ntoken  addwhitelist '["'"${show_con}"'"]'   -p $ticket_ntoken

mpush $show_con init '["flonian", "cvticket.nft"]' -p $show_con

mpush $show_con addshowadm '["flonian"]' -p flonian

mpush $show_con addshowadm '["myadmin"]' -p flonian

mpush $show_con addshowadm '["testtest"]' -p flonian

mpush $show_con addshowadm '["fulgwxvwfw1m"]' -p flonian
mpush $show_con addshowadm '["flontest"]' -p flonian


#普通票
mpush $show_con nftcreate '[
  10000000000,{"value": 21010001000010021},
  "ipfs://ticket/silver11.json"
]' -p flonian


mpush $show_con nftissue '[
  {"amount": 2000, "symbol": {"value": 21010001000010021} },
  "bootstrap batch"
]' -p flonian


#合影票
mpush $show_con nftcreate '[
  10000000000,{"value": 21010001000010022},
  "ipfs://ticket/silver12.json"
]' -p flonian


mpush $show_con nftissue '["flonian",{"amount": 11, "symbol": {"value": 21010001000010128} },"issue:20250053"]' -p flonian


mpush $show_con newshow '[41258,"concert", true, true, "2025-09-01T00:00:00", "2025-09-10T23:59:59","test" ,"gasgsgg gq gg sa gdf jhfd j" , "onshelf"]' -p flonian
mpush $show_con addchecker '[41258,"myadmin"]'   -p flonian

mpush $show_con newshow '[41259,"concert", true, true, "2025-09-01T00:00:00", "2025-09-10T23:59:59","test" ,"gasgsgg gq gg sa gdf jhfd j" , "onshelf"]' -p flonian
mpush $show_con addchecker '[41259,"myadmin"]'   -p flonian

mpush $show_con newshow '[41260,"concert", true, true, "2025-09-01T00:00:00", "2025-09-10T23:59:59","test" ,"gasgsgg gq gg sa gdf jhfd j" , "onshelf"]' -p flonian
mpush $show_con addchecker '[41260,"myadmin"]'   -p flonian
mpush $show_con addchecker '[41260,"flonian"]'   -p flonian


mpush $show_con addchecker '[1757054921640,"cvohdzu2awu4"]'   -p flonian
mpush $show_con addchecker '[1757382785471,"cvohdzu2awu4"]'   -p flonian





mpush $show_con newticket '[41258, {"value": 21010001000010021}, {"value": 0}, "普通票", "100.00 USDT", 100, "running", "2025-08-01T00:00:00", "2025-09-10T23:59:59"]' -p flonian
mpush $show_con newticket '[41258, {"value": 21010001000010022}, {"value": 21010001000010021}, "VIP", "200.00 USDT", 100, "running", "2025-08-01T00:00:00", "2025-09-10T23:59:59"]' -p flonian
mpush $show_con newticket '[41258, {"value": 21010001000010023}, {"value": 21010001000010022}, "VIP1", "200.00 USDT", 100, "running", "2025-08-01T00:00:00", "2025-09-10T23:59:59"]' -p flonian
mpush $show_con newticket '[41258, {"value": 21010001000010024}, {"value": 21010001000010022}, "VIP2", "200.00 USDT", 100, "running", "2025-08-01T00:00:00", "2025-09-10T23:59:59"]' -p flonian
mpush $show_con newticket '[41258, {"value": 21010001000010025}, {"value": 21010001000010022}, "VIP3", "200.00 USDT", 100, "running", "2025-08-01T00:00:00", "2025-09-10T23:59:59"]' -p flonian
mpush $show_con newticket '[41258, {"value": 21010001000010026}, {"value": 21010001000010022}, "VIP4", "200.00 USDT", 100, "running", "2025-08-01T00:00:00", "2025-09-10T23:59:59"]' -p flonian
mpush $show_con newticket '[41258, {"value": 21010001000010027}, {"value": 21010001000010022}, "VIP5", "200.00 USDT", 100, "running", "2025-08-01T00:00:00", "2025-09-10T23:59:59"]' -p flonian


mpush $show_con issue '["gahbnbehaskk",41258,21010001000010021,1,"send to gahbnbehaskk 1 ticket NFT"]' -p flonian



# 批量赠票：show_id=1, ticket_id=101，memo=批量赠票测试

mpush $show_con giftbatch '["flonian",
  20250057,
  21010001000010137,
  1,
  ["myadmin","testtest","fulgwxvwfw1m","flontest"],
  "batch gift issue"
]' -p flonian