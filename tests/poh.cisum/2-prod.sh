#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

poh_con=poh.cisum
mreg flon $poh_con flonian
mtran flonian $poh_con "100 FLON"
mset $poh_con poh.cisum
mcli set account permission $poh_con active --add-code

cisum_token=cisum.token
mpush $cisum_token  addwhitelist '["poh.cisum"]'   -p $cisum_token

max_issued="2500000000.00000000 SING"
mpush $poh_con init '["cisumreserve","cisum.admin","'"$max_issued"'"]' -p $poh_con



#mpush $poh_con updateissued '[]' -p $poh_con



mpush cisum.token transfer '["flonian","poh.cisum","400.0000 CISUM","refuel:200:1763351493:1764197165"]' -p flonian


mpush sing.token transfer '["flonian","poh.cisum","300.00000000 SING","refuel:tests:200:1763366914:1764197165"]' -p flonian


mpush poh.cisum setfundtime '["flonian","8,SING","sing.token",1763350138,null]' -p flonian

mpush poh.cisum redeemfund '["gahbnbehaskk","flonian"]' -p gahbnbehaskk

mpush poh.cisum redeemfund '["flonian","flonian"]' -p flonian
