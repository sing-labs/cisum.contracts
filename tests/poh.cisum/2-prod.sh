#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

poh_con=poh.cisum
mreg flon $poh_con flonian
mtran flonian $poh_con "100 FLON"
mset $poh_con poh.cisum
mcli set account permission $poh_con active --add-code

nestar_token=song.token
mpush $nestar_token  addwhitelist '["poh.cisum"]'   -p $nestar_token

# 给合约开 CISUM 余额行（RAM 自付）
mpush sing.token open '[
  "'"${poh_con}"'",
  "8,SING",
  "'"${poh_con}"'"
]' -p $poh_con

max_issued="2500000000.00000000 SING"
mpush $poh_con init '["cisumreserve","flonian","'"$max_issued"'"]' -p $poh_con



#mpush $poh_con registreward '[registrar11,mywallet2,"gahbnbehaskk"]' -p registrar11


