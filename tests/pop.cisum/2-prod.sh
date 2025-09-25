#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

pop_con=pop.cisum
mreg flon $pop_con flonian
mtran flonian $pop_con "100 FLON"
mset $pop_con pop.cisum
mcli set account permission $pop_con active --add-code


#向pop_con 转入sing
mpush sing.token transfer '[
  "flonian",
  "'"${pop_con}"'",
  "1000.00000000 SING",
  "seed for pop rewards"
]' -p flonian

mpush $pop_con addexecutor '["show.cisum"]' -p $pop_con


badgestore_contract=badgecvstore
mpush $badgestore_contract addwhitelist '["pop.cisum"]' -p $badgestore_contract

# 新增规则（id=0 表示新增）
mpush $pop_con  setbrule '[0, "50.000000 USDT", {"value":"4299369637478511"}, true]' -p $pop_con
mpush $pop_con  setbrule '[0, "500.000000 USDT", {"value":"4299369637478512"}, true]' -p $pop_con
mpush $pop_con  setbrule '[0, "5000.000000 USDT", {"value":"4299369637478513"}, true]' -p $pop_con
mpush $pop_con  setbrule '[0, "50000.000000 USDT", {"value":"4299369637478514"}, true]' -p $pop_con
