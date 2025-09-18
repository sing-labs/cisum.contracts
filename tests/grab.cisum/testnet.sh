#!/bin/bash
set -e

## 假定：这些环境变量已在外部导出
## rolemanage_con=rolemanage13
## nestar_token=nest21.token

grab_con=grab23.cisum

# 1) 账户准备
mreg flon $grab_con flonian
mtran flonian $grab_con "100 FLON"
mset  $grab_con grab.cisum
mcli set account permission $grab_con active --add-code

# 2) rolemanage：准备角色&授权（方案A所需）
# （如未创建角色，可先打开这两行）
# mpush $rolemanage_con setrole '["admin","ops admin role"]'   -p flonian
# mpush $rolemanage_con setrole '["oracle","ops oracle role"]' -p flonian

# 把 myadmin 在【contract=grab23.cisum】作用域授为 admin / oracle
mpush $rolemanage_con grantrole '["grab23.cisum","flonian","myadmin","admin"]'  -p flonian
mpush $rolemanage_con grantrole '["grab23.cisum","flonian","myadmin","oracle"]' -p flonian

mpush $rolemanage_con grantrole '["grab23.cisum","flonian","flonian","admin"]'  -p flonian
mpush $rolemanage_con grantrole '["grab23.cisum","flonian","flonian","oracle"]' -p flonian


# 让 grab 合约能作为“查询方”访问 rolemanage（闸门：submitter是self或admin/oracle）
mpush $rolemanage_con addoracle '["grab23.cisum"]' -p flonian

# 3) grab 合约初始化（允许合约自签）
mpush $grab_con init '["flonian"]' -p $grab_con

# 4) 业务相关白名单
mpush $nestar_token  addconsumewl '["grab23.cisum"]' -p $nestar_token
mpush  cvticket.nft  addwhitelist '["grab23.cisum"]' -p cvticket.nft

# 5) grab 的配置项：方案A下由具备角色的用户（myadmin）提交（带 submitter）
mpush grab23.cisum cfgpoint  '["flonian","nest21.token"]' -p myadmin
mpush grab23.cisum cfgticket '["myadmin","cvticket.nft"]' -p myadmin

# 允许的支付币种（该 action 仍是合约自签）
mpush grab23.cisum settoken '["4,NESTAR","nest21.token"]' -p grab23.cisum


# 7)（示例）删除某 rush_sale（新签名需要 submitter；可按需保留/删除）
# numbers 不要加引号，bool 用 true/false
mpush grab23.cisum delrushsale '["myadmin",	10,true]' -p myadmin

# 8) 抢购支付样例（不变）
mpush nestar.token transfer '["gahbnbehaskk","grab.cisum","200.0000 NESTAR","grab:2:a1799ae8e1ea62a20a5f56710342a401"]' -p gahbnbehaskk
mpush nest21.token transfer '["ipowner.111","grab23.cisum","200.0000 NESTAR","grab:16:a1799ae8e1ea62a20a5f56810342a401"]' -p ipowner.111
mpush nest21.token transfer '["myadmin","grab23.cisum","200.0000 NESTAR","grab:16:a1799ae8e1ea62a20a5f56910342a401"]' -p myadmin
mpush nest21.token transfer '["nes11.issuer","grab23.cisum","200.0000 NESTAR","grab:16:a1799ae8e1ea62a20a5f57010342a401"]' -p nes11.issuer

# 9) 压测循环（不变）
for i in {1..1000}; do
  echo "第 $i 次执行..."
  mpush nest21.token transfer '["gahbnbehaskk","grab23.cisum","200.0000 NESTAR","grab:3:a1799ae8e1ea62a20a5f56710342a501"]' -p gahbnbehaskk
  sleep 0.5
done

# 10) 修改 rush_sale（已带 submitter，方案A）
mpush grab23.cisum setrushsale '["myadmin",10,10,1000,"2025-09-01T20:00:00"]' -p myadmin

# 11) 结束后清理活动（已带 submitter，方案A）
mpush grab23.cisum clearsale '["myadmin",10]' -p myadmin


mpush grab.cisum setrushsale '["flonian",3,1000,4000,"2025-10-01T20:00:00"]' -p flonian