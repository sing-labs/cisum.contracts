show_con=show24.cisum
mreg flon $show_con flonian
mtran flonian $show_con "100 FLON"
mset $show_con show.cisum
mcli set account permission $show_con active --add-code



mpush ticket.cvnft  addwhitelist '["'"${show_con}"'"]'   -p ticket.cvnft

mpush $show_con init '["flonian", "ticket.cvnft"]' -p $show_con

mpush $show_con addshowadm '["flonian","myadmin"]' -p flonian

mpush $show_con addshowadm '["myadmin"]' -p flonian

mpush $show_con addshowadm '["testtest"]' -p flonian

mpush $show_con addshowadm '["fulgwxvwfw1m"]' -p flonian
mpush $show_con addshowadm '["flontest"]' -p flonian







#普通票
mpush $show_con createnft '[
  10000000000,{"value": 21010001000010021},
  "ipfs://ticket/silver11.json"
]' -p flonian


mpush $show_con issuenft '[
  {"amount": 2000, "symbol": {"value": 21010001000010021} },
  "bootstrap batch"
]' -p flonian


#合影票
mpush $show_con createnft '[
  10000000000,{"value": 21010001000010022},
  "ipfs://ticket/silver12.json"
]' -p flonian


mpush $show_con issuenft '["flonian",{"amount": 11, "symbol": {"value": 21010001000010241} },"issue:20250064"]' -p flonian


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


mpush $show_con issue '["gahbnbehaskk",1757500247480,2101025469022,1,"send to gahbnbehaskk 1 ticket NFT"]' -p flonian



# 批量赠票：show_id=1, ticket_id=101，memo=批量赠票测试

mpush $show_con giftbatch '["flonian",
  20250057,
  21010001000010137,
  3,
  ["myadmin","testtest","fulgwxvwfw1m","flontest"],
  "batch gift issue"
]' -p flonian


mpush $pop_con addexecutor '["show24.cisum"]' -p $pop_con
mpush show24.cisum addplatadm '["myadmin"]'  -p flonian

mpush show.cisum buyticket '["flonian","cvph53ao15sq", "20.000000 USDT", 1758268451780, 2101025696004, 1, "1968952961493520384"]' -p flonian


mpush show.cisum setticket '["flonian",1758870677786,2101025150959,"vip","100000 VND","10 USDT","2025-09-26T07:03:17.000","2025-11-30T18:00:00"]' -p flonian

mpush show.cisum setticket '["flonian",1758870677786,2101025186415,"dinner","500000 VND","50 USDT","2025-09-26T08:09:44.000","2025-11-30T18:00:00"]' -p flonian

mpush show.cisum setticket '["flonian",1758870677786,2101025194952,"vip","300000 VND","30 USDT","2025-09-26T08:06:13.000","2025-11-30T18:00:00"]' -p flonian

mpush show.cisum setticket '["flonian",1758870677786,2101025468200,"free","100.0000 CISUM","0.0000 USDT","2025-09-26T07:00:48.000","2025-11-30T18:00:00"]' -p flonian

mpush show.cisum setticket '["flonian",1758870677786,2101025538250,"photo_op","450000 VND","45 USDT","2025-09-26T08:08:34.000","2025-11-30T18:00:00"]' -p flonian

mpush show.cisum setticket '["flonian",1758870677786,2101025748071,"vip","150000 VND","15 USDT","2025-10-01T08:05:17.000","2025-11-30T18:00:00"]' -p flonian

mpush show.cisum setticket '["flonian",1758870677786,2101025906313,"afterparty","350000 VND","35 USDT","2025-09-26T08:07:42.000","2025-11-30T18:00:00"]' -p flonian


mpush show.cisum setshow '["flonian",1758870677786,"concert",false,false,"2025-11-30T18:00:00","2025-11-30T23:00:00","Vũ Thanh Vân Concert Việt Nam",""]' -p flonian

mpush show.cisum nftissue '["flonian","show.cisum",{"amount":100,"symbol":{"value":2101025468200}},"issue:1758870677786"]' -p show.cisum


mpush show.cisum nftissue '["flonian","flonian",{"amount":100,"symbol":{"value":2101025468200}},"issue:1758870677786"]' -p flonian

mpush show.cisum nftissue \
'["flonian","flonian",{"amount":100,"symbol":{"value":2101025468200}},"issue:1758870677786"]' \
-p flonian


 mcli get table show.cisum show.cisum tickets



 mpush $show_con nftissue '["show.cisum","show.cisum",{"amount": 11, "symbol": {"value": 2101025468200} },"issue:1758870677786"]' -p show.cisum


 mpush $show_con issuetograb '[
   "show.cisum",
   "grab.cisum",
   {"amount": 100, "symbol": {"value": 2101025468200} },
   "addrushsale:23:1758870677786"]' -p show.cisum



mpush $show_con delshow '["flonian",20250289]' -p flonian