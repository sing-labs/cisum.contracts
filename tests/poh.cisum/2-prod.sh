poh_con=poh.cisum
mreg flon $poh_con flonian
mtran flonian $poh_con "100 FLON"
mset $poh_con poh.cisum
mcli set account permission $poh_con active --add-code



mpush $nestar_token  addwhitelist '["poh.cisum"]'   -p $nestar_token

# 给合约开 CISUM 余额行（RAM 自付）
mpush cisum.token open '[
  "'"${poh_con}"'",
  "8,CISUM",
  "'"${poh_con}"'"
]' -p $poh_con


mpush $poh_con init '["cisumreserve","flonian","2500000000.00000000 CISUM"]' -p $poh_con



#mpush $poh_con registreward '[registrar11,mywallet2,"gahbnbehaskk"]' -p registrar11


