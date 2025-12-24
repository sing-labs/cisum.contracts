cisum_token=sing.token
mreg flon $cisum_token flonian
mtran flonian $cisum_token "100 FLON"
mset $cisum_token cisum.token
mcli set account permission $cisum_token active --add-code



mpush $cisum_token create '["flonian","10000000000.00000000 CISUM"]' -p $cisum_token
mpush $cisum_token issue '["flonian","100000000.00000000 CISUM","1st issue"]' -p flonian
mtran -c $cisum_token flonian flon.xchain "1000000.00000000 CISUM" "refuel"
mcli get currency stats $cisum_token CISUM
mcli get currency balance $cisum_token flonian CISUM




mpush $cisum_token create '["flonian","10000000000.00000000 MUSIC"]' -p $cisum_token
mpush $cisum_token issue '["flonian","100000000.00000000 MUSIC","1st issue"]' -p flonian
mtran -c $cisum_token flonian flon.xchain "1000000.00000000 MUSIC" "refuel"
mcli get currency stats $cisum_token MUSIC
mcli get currency balance $cisum_token flonian MUSIC




#mpush $cisum_token transfer '["flonian", "pos13.cisum", "160000.00000000 MUSIC", "airdrop"]' -p flonian

#mpush $cisum_token transfer '["gahbnbehaskk", "flonian", "60000.00000000 MUSIC", "airdrop"]' -p gahbnbehaskk


#mpush sing.token transfer '["flonian", "n3hwf4utqo5c", "2000.00000000 SING", "airdrop"]' -p flonian
#mpush flon.token transfer '["flonian", "n3hwf4utqo5c", "2000.00000000 FLON", "airdrop"]' -p flonian

#mpush flon.mtoken transfer '["flonian", "n3hwf4utqo5c", "2000.000000 USDT", "airdrop"]' -p flonian

mpush sing.token transfer '["flonian", "cv1anal1acjh", "100000.00000000 SING", "airdrop"]' -p flonian

mpush sing.token transfer '["gahbnbehaskk", "flonian", "2000.00000000 SING", "airdrop"]' -p gahbnbehaskk


mpush sing.token transfer '["flonian", "mywallet2", "4000.00000000 SING", "airdrop"]' -p flonian


