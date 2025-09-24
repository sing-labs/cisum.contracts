#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc




# 1) 账户准备
grab_con=grab.cisum
mreg flon $grab_con flonian
mtran flonian $grab_con "100 FLON"
mset  $grab_con grab.cisum
mcli set account permission $grab_con active --add-code

# 把 flonian 在【contract=grab.cisum】作用域授为 admin
mpush $rolemanage_con grantrole '["grab.cisum","flonian","flonian","admin"]'  -p flonian
# grab 合约初始化（允许合约自签）
admin=flonian
mpush $grab_con init '["'"$admin"'"]' -p $grab_con

nestar_token=nestar.cisum
mpush $nestar_token  addconsumewl '["grab.cisum"]' -p $nestar_token
mpush  ticket.cvnft addwhitelist '["grab.cisum"]' -p ticket.cvnft

# 5) grab 的配置项：方案A下由具备角色的用户（myadmin）提交（带 submitter）
mpush $grab_con cfgpoint  '["flonian","nestar.token"]' -p $grab_con
mpush $grab_con cfgticket '["flonian","ticket.cvnft"]' -p $grab_con

mpush $grab_con addtoken '["4,SONG","nestar.token"]' -p $grab_con

#mpush $grab_con settoken '["6,USDT","flon.mtoken"]' -p $grab_con
#mpush $grab_con settoken '["8,CISUM","cisum.token"]' -p $grab_con
#mpush $grab_con deltoken '["6,USDT","flon.mtoken"]' -p $grab_con
#mpush $grab_con deltoken '["8,CISUM","cisum.token"]' -p $grab_con
#mpush $grab_con deltoken '["4,NESTAR","nestar.token"]' -p $grab_con