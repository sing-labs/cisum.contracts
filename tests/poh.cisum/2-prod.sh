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


