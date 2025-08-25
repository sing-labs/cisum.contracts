pop_con=pop11.cisum
mreg flon $pop_con flonian
mtran flonian $pop_con "100 FLON"
mset $pop_con pop.cisum
mcli set account permission $pop_con active --add-code


#向pop_con 转入CISUM
mpush cisum.token transfer '[
  "flonian",
  "'"${pop_con}"'",
  "1000.00000000 CISUM",
  "seed for pop rewards"
]' -p flonian

#myadmin消费了100.000000 USDT ，发放奖励1001 CISUM
mpush $pop_con mine '["myadmin","100.000000 USDT","order Reward:1001"]' -p $pop_con


