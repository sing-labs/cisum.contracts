nestar_token=nest21.token
mreg flon $nestar_token flonian
mtran flonian $nestar_token "100 FLON"
mset $nestar_token nestar.token
mcli set account permission $nestar_token active --add-code



# 创建 NESTAR 发行者账号
issuer_owner=nes11.issuer
mreg flon $issuer_owner flonian
mtran flonian $issuer_owner "100 FLON"


# 创建普通账号
user_owner=gahbnbehaskk
mreg flon $user_owner flonian
mtran flonian $user_owner "100 FLON"

# 创建admin
user_admin=myadmin
mreg flon $user_admin flonian
mtran flonian $user_admin "100 FLON"



#艺人合约
artist_contract=art14.token



mpush $nestar_token setissuer '["'"${issuer_owner}"'"]' -p $nestar_token

mpush $nestar_token setcontract '["'"${artist_contract}"'"]' -p $nestar_token

mpush $nestar_token setadmin '["'"${user_admin}"'"]' -p $nestar_token

#设置勋章合约
badgestore_contract=badgestore11
mpush $nestar_token setbadgestore '["'"${badgestore_contract}"'"]' -p $nestar_token


# 参数：issuer, maximum_supply
mpush $nestar_token create '["'"${issuer_owner}"'", "10000000000000.0000 NESTAR"]' -p $nestar_token

mpush $nestar_token issue '["'"${issuer_owner}"'", "1000000000.0000 NESTAR", "bootstrap"]' -p $issuer_owner


# 新增规则（id=0 表示新增）
mpush $nestar_token  setbrule '[0, "15000.0000 NESTAR", {"value":"4299369637478511"}, true]' -p $nestar_token
mpush $nestar_token  setbrule '[0, "150000.0000 NESTAR", {"value":"4299369637478512"}, true]' -p $nestar_token
mpush $nestar_token  setbrule '[0, "1500000.0000 NESTAR", {"value":"4299369637478513"}, true]' -p $nestar_token
mpush $nestar_token  setbrule '[0, "15000000.0000 NESTAR", {"value":"4299369637478514"}, true]' -p $nestar_token


mpush $nestar_token delbrule '["0"]' -p $nestar_token






mpush $nestar_token transfer '["'"${issuer_owner}"'", "gahbnbehaskk", "500.0000 NESTAR", "airdrop"]' -p $issuer_owner

mpush $nestar_token transfer '["'"${issuer_owner}"'", "ipowner.111", "500.0000 NESTAR", "airdrop"]' -p $issuer_owner

mpush $nestar_token transfer '["gahbnbehaskk", "ipowner.111", "5000.0000 NESTAR", "airdrop"]' -p gahbnbehaskk

#向艺人转账
mpush $nestar_token transfer '["gahbnbehaskk", "artist.acc1", "150000.0000 NESTAR", "airdrop"]' -p gahbnbehaskk


# 6) 用户给艺人打赏（需艺人已白名单）
mpush $nestar_token transfer '["ipowner.111", "artist.acc1", "15000.0000 NESTAR", "tip"]' -p ipowner.111



#设置cisum 转化为 NESTAR 的汇率
mpush $nestar_token setrate '["100"]' -p $nestar_token



mpush $nestar_token transfer '["gahbnbehaskk", "grab23.cisum", "300.0000 NESTAR", "airdrop"]' -p gahbnbehaskk

issuer_owner=nes11.issuer
nestar_token=nestar.token
mpush $nestar_token transfer '["flonian", "cvph53ao15sq", "2000.0000 SONG", "airdrop"]' -p flonian



mpush cisum.token transfer '["flonian", "cvph53ao15sq", "2000.00000000 SING", "airdrop"]' -p flonian

