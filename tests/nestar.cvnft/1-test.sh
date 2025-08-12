nestar_token=nest11.token
mreg flon $nestar_token flonian
mtran flonian $nestar_token "100 FLON"
mset $nestar_token nestar.cisum
mcli set account permission $nestar_token active --add-code



#创建艺人账号
artister_owner=artist.acc1
mreg flon $artister_owner flonian
mtran flonian $artister_owner "100 FLON"



# 创建 NESTAR 发行者账号
issuer_owner=nes11.issuer
mreg flon $issuer_owner flonian
mtran flonian $issuer_owner "100 FLON"


# 创建普通账号
user_owner=gahbnbehaskk
mreg flon $user_owner flonian
mtran flonian $user_owner "100 FLON"

 



mpush $nestar_token setissuer '["'"${issuer_owner}"'"]' -p $nestar_token

# 参数：issuer, maximum_supply
mpush $nestar_token create '["'"${issuer_owner}"'", "10000000000.0 NESTAR"]' -p $nestar_token



mpush $nestar_token issue '["'"${issuer_owner}"'", "1000000.0 NESTAR", "bootstrap"]' -p $issuer_owner


mpush $nestar_token setartist '["'"${artister_owner}"'", true]' -p $nestar_token

mpush $nestar_token transfer '["'"${issuer_owner}"'", "gahbnbehaskk", "500.0 NESTAR", "airdrop"]' -p $issuer_owner


mpush nestar.test4 transfer '["mywallet2","ipowner.111","500 NESTAR","airdrop"]' -p mywallet2

mcli get account mywallet2



# 6) 用户给艺人打赏（需艺人已白名单）
mpush $nestar_token transfer '["ipowner.111", "'"${issuer_owner}"'", "100 NESTAR", "tip"]' -p ipowner.111



