#!/bin/bash

grab_con=grab.cisum

# 1) 账户准备
mreg flon $grab_con flonian
mtran flonian $grab_con "100 FLON"
mset  $grab_con grab.cisum
mcli set account permission $grab_con active --add-code


# 把 flonian 在【contract=grab.cisum】作用域授为 admin
mpush $rolemanage_con grantrole '["grab.cisum","flonian","flonian","admin"]'  -p flonian


# grab 合约初始化（允许合约自签）
mpush $grab_con init '["flonian"]' -p $grab_con

# 4) 业务相关白名单
nestar_token=nestar.cisum
mpush $nestar_token  addconsumewl '["grab.cisum"]' -p $nestar_token
mpush  ticket.cvnft addwhitelist '["grab.cisum"]' -p ticket.cvnft

# 5) grab 的配置项：方案A下由具备角色的用户（myadmin）提交（带 submitter）
mpush $grab_con cfgpoint  '["flonian","nestar.token"]' -p $grab_con
mpush $grab_con cfgticket '["flonian","ticket.cvnft"]' -p $grab_con

# 允许的支付币种（该 action 仍是合约自签）
mpush $grab_con settoken '["4,NESTAR","nestar.token"]' -p $grab_con





mpush nestar.token transfer '["gahbnbehaskk","grab.cisum","200.0000 NESTAR","grab:4:a1799ae8e1ea62a20a5f56710342a115"]' -p gahbnbehaskk