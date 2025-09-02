cisum_token=cisum.token
mreg flon $cisum_token flonian
mtran flonian $cisum_token "100 FLON"
mset $cisum_token flon.token
mcli set account permission $cisum_token active --add-code



mpush $cisum_token create '["flonian","10000000000.00000000 CISUM"]' -p $cisum_token
mpush $cisum_token issue '["flonian","100000000.00000000 CISUM","1st issue"]' -p flonian
mtran -c $cisum_token flonian flon.xchain "1000000.00000000 CISUM" "refuel"
mcli get currency stats $cisum_token CISUM
mcli get currency balance $cisum_token flonian CISUM