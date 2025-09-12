pop_con=pop14.cisum
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

#报错
mpush $pop_con mine '["myadmin","-100.000000 USDT","order Reward:1001"]' -p $pop_con

#报错
mpush $pop_con mine '["myadmin","-0.000000 USDT","order Reward:1001"]' -p $pop_con
#>> [pop] reward=0, skip mint for myadmin
mpush $pop_con mine '["myadmin","0.000001 USDT","order Reward:1001"]' -p $pop_con

#
mpush $pop_con mine '["myadmin","0.005001 USDT","order Reward:1001"]' -p $pop_con


mpush $pop_con mine '["myadmin","0.000010 USDT","order Reward:1001"]' -p $pop_con



mpush $pop_con addexecutor '["myadmin"]' -p $pop_con

mpush $pop_con addexecutor '["show24.cisum"]' -p $pop_con


mpush $pop_con mine '["myadmin","0.000010 USDT","order Reward:1001"]' -p myadmin