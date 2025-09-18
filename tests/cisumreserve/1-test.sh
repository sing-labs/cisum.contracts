
admin=myadmin

cisumreserve_con=cisumreser11
mreg flon $cisumreserve_con flonian
mtran flonian $cisumreserve_con "100 FLON"
mset $cisumreserve_con cisumreserve
mcli set account permission $cisumreserve_con active --add-code


updateauth

# ==== 1) 初始化（只需一次）====
# fee_bps 例：50 = 0.50%
mpush $cisumreserve_con init '["'"$admin"'", 30]' -p $cisumreserve_con

# ==== 2) 修改费率（管理员签）====
# 把费率改为 0.25%
mpush $cisumreserve_con setfee '["'"$admin"'", 30]' -p $admin

# ==== 3) 变更管理员（旧管理员或合约自签）====
mpush $cisumreserve_con setadmin '["'"$admin"'", "flonian"]' -p $admin

mpush $cisumreserve_con setadmin '["flonian", "'"$admin"'"]' -p flonian
# 之后管理操作请用新管理员签：reserveadmin

# ==== 4) （可选）给储备池合约预置 CISUM 库存 ====
# 注意：issuer 需要是真正持仓的账户
mpush cisum.token transfer '["flonian","cisumreser11","100.00000000 CISUM","seed reserve"]' -p flonian

# ==== 5) （联调）用户充值 USDT 触发发币 ====
# from = alice，first_receiver 必须是 flon.mtoken；金额 6 位小数
mpush flon.mtoken transfer '["gahbnbehaskk","cisumreser11","1.000000 USDT","buy CISUM"]' -p gahbnbehaskk

# ==== 6)（可选）再改费率、再测一笔 ====
mpush $cisumreserve_con setfee '["reserveadmin", 100]' -p reserveadmin     # 1.00%
mpush flon.mtoken transfer '["bob","'"$cisumreserve_con"'","50.000000 USDT","buy CISUM"]' -p bob



mpush flon.mtoken transfer '["flonian","gahbnbehaskk","100.000000 USDT","buy CISUM"]' -p flonian
