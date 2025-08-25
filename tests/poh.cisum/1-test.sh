poh_con=poh12.cisum
mreg flon $poh_con flonian
mtran flonian $poh_con "100 FLON"
mset $poh_con poh.cisum
mcli set account permission $poh_con active --add-code


# 给合约开 CISUM 余额行（RAM 自付）
mpush cisum.token open '[
  "'"${poh_con}"'",
  "8,CISUM",
  "'"${poh_con}"'"
]' -p $poh_con

# 给合约充值奖励池（示例从 flonian 转入）
mpush cisum.token transfer '[
  "flonian",
  "'"${poh_con}"'",
  "1000000.00000000 CISUM",
  "seed for PoH rewards"
]' -p flonian


mpush $poh_con init '["cisumplatfrm","registrar11"]' -p $poh_con

mpush $poh_con setplatform '["cisumplatfrm"]' -p $poh_con



mpush $poh_con setregistrar '["registrar11"]' -p $poh_con


mpush $poh_con setrewards '["2550000000.00000000 CISUM"]' -p $poh_con


mpush $poh_con registreward '["registrar11","PoH signup reward"]' -p registrar11