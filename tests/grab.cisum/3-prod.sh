#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc




# 1) 账户准备
grab_con=grab.cisum
mreg flon $grab_con flonian
mtran flonian $grab_con "100 FLON"
mset  $grab_con grab.cisum
mcli set account permission $grab_con active --add-code

# grab 合约初始化（允许合约自签）
admin=flonian
mpush $grab_con init '["'"$admin"'"]' -p $grab_con

cisum_token=cisum.token
mpush $cisum_token  addconsumewl '["grab.cisum"]' -p $cisum_token
mpush  ticket.cvnft addwhitelist '["grab.cisum"]' -p ticket.cvnft

# 5) grab 的配置项：方案A下由具备角色的用户（myadmin）提交（带 submitter）
mpush $grab_con cfgpoint  '["flonian","cisum.token"]' -p $grab_con
mpush $grab_con cfgticket '["flonian","ticket.cvnft"]' -p $grab_con

mpush $grab_con addtoken '["4,CISUM","cisum.token"]' -p $grab_con

# 6) grab 的 oracle 账号
oracle=
mpush $grab_con addoracle '["'"$oracle"'"]' -p $grab_con






mpush $grab_con addupgrade '["flonian",1760855632324,2101025077573,"2025-10-20T00:00:00","2025-11-20T00:00:00",{ "amount": 1, "symbol": { "value": "2101025919776" } },10,5000]' -p flonian

mpush ticket.cvnft transfer '["flonian","grab.cisum",[{"amount":1,"symbol":{"value":2101025921799}}],"rushupgrade:1:a1799ae8e1ea72a20a5f57710352a123"]' -p flonian


mpush cisum.token transfer '["flonian","grab.cisum","100.0000 CISUM","grab:40:a1799ae8e1ea72a20a5f56710642a143"]' -p flonian



show_con=show.cisum
mpush $show_con issuetograb '["show.cisum","grab.cisum", {"amount":100,"symbol":{"value":'2101025077573'}}, "addrushupgrade:2:1760855632324"]' -p $show_con

mpush grab.cisum delupgrade '["flonian",2,true]' -p flonian




mpush grab.cisum setrushsale '["flonian",29,"200.0000 CISUM",null,null,null,null]' -p flonian
mpush grab.cisum setupgrade '["flonian",28,null,1000,"2025-10-01T20:00:00.000"]' -p flonian



mpush grab.cisum clearupgrade '["flonian",1]' -p flonian


mpush grab.cisum delrushorder '["flonian",28]' -p flonian


mpush cisum.token transfer '["flonian","grab.cisum","100.0000 CISUM","grab:23:a1799ae8e1ea72a20a5f577106422143"]' -p flonian


admin=cisumverseop
mpush $grab_con init '["'"$admin"'",55,3000]' -p $grab_con

# 5) grab 的配置项：方案A下由具备角色的用户（myadmin）提交（带 submitter）
mpush $grab_con cfgpoint  '["cisum.token"]' -p $grab_con
mpush $grab_con cfgticket '["ticket.cvnft"]' -p $grab_con


# 6) grab 的 oracle 账号
oracle=cisumgrabops
grab_con=grab.cisum
mpush $grab_con addoracle '["'"$oracle"'"]' -p $grab_con



mpush cisum.token transfer '["flonian","grab.cisum","100.0000 CISUM","grab:80:a1799ae8e1ea72a20a5f577106442143"]' -p flonian






mpush grab.cisum delrushsale '["cisumverseop",16,true]' -p cisumverseop
mpush grab.cisum delrushsale '["cisumverseop",14,true]' -p cisumverseop
mpush grab.cisum delrushsale '["cisumverseop",17,true]' -p cisumverseop
mpush grab.cisum delrushsale '["cisumverseop",15,true]' -p cisumverseop
mpush grab.cisum delrushsale '["cisumverseop",23,true]' -p cisumverseop


mpush grab.cisum delrushsale '["flonian",3,true]' -p flonian

mpush grab.cisum delupgrade '["flonian",23,true]' -p flonian





mpush grab.cisum setrushsale '["ful4oe5culzv",18,null,null,null,"2026-01-16T15:00:00.000",null]' -p ful4oe5culzv
mpush grab.cisum setrushsale '["ful4oe5culzv",19,null,null,null,"2026-01-16T15:00:00.000",null]' -p ful4oe5culzv
mpush grab.cisum setrushsale '["ful4oe5culzv",20,null,null,null,"2026-01-16T15:00:00.000",null]' -p ful4oe5culzv


mpush show.cisum setshow '["ful4oe5culzv",1768217670078,"concert",false,false,"2026-04-12T21:00:00.000","2026-04-12T21:00:00.000","2026 2Z GloryDayz in Brazil",""]' -p ful4oe5culzv





mpush grab.cisum setrushsale '["ful4oe5culzv",22,null,null,null,"2026-01-16T15:00:00.000",null]' -p ful4oe5culzv
mpush grab.cisum setrushsale '["ful4oe5culzv",23,null,null,null,"2026-01-16T15:00:00.000",null]' -p ful4oe5culzv
mpush grab.cisum setrushsale '["ful4oe5culzv",24,null,null,null,"2026-01-16T15:00:00.000",null]' -p ful4oe5culzv
mpush show.cisum setshow '["ful4oe5culzv",1768372003091,"concert",false,false,"2026-04-12T21:00:00.000","2026-04-12T21:00:00.000","2026 2Z GloryDayz in Brazil",""]' -p ful4oe5culzv


mpush show.cisum setticket '["ful4oe5culzv",1768372003091,2101025045293,"free","0.0000 BRL","0.0000 USDT","2026-01-16T15:00:00","2026-04-12T22:00:00"]' -p ful4oe5culzv
mpush show.cisum setticket '["ful4oe5culzv",1768372003091,2101025483997,"free","0.0000 BRL","0.0000 USDT","2026-01-16T15:00:00","2026-04-12T22:00:00"]' -p ful4oe5culzv
mpush show.cisum setticket '["ful4oe5culzv",1768372003091,2101025894255,"free","0.0000 BRL","0.0000 USDT","2026-01-16T15:00:00","2026-04-12T22:00:00"]' -p ful4oe5culzv


mpush show.cisum delshow '["ful4oe5culzv","1768217670078"]' -p ful4oe5culzv



#从抢票功能中减少票
mpush grab.cisum subgrab '["flonian","rushsale",30,{"amount":1,"symbol":{"nid":21010001000030810}}]' -p flonian

mpush grab.cisum subgrab '["flonian","rushupgrade",60,{"amount":1,"symbol":{"nid":2101025014097}}]' -p flonian
