sbt_owner=sbt.test4
mreg flon $sbt_owner flonian
mtran flonian $sbt_owner "100 FLON"
mset $sbt_owner sbt.cvnft
mcli set account permission $sbt_owner active --add-code



ip_owner=ipowner.111
mreg flon $ip_owner flonian
mtran flonian $ip_owner "100 FLON"
mset $ip_owner sbt.cvnft
mcli set account permission $ip_owner active --add-code


#创建公证人合约账户
notary_owner=notary.111
mreg flon $notary_owner flonian
mtran flonian $notary_owner "100 FLON"




# 参数：issuer, maximum_supply, nsymbol{id,pid}, token_uri, ipowner
mpush $sbt_owner create '["mywallet2", 1000000000, {"id":1,"pid":512}, "", "'"${ip_owner}"'"]' -p mywallet2


# 空投白名单（允许这些外部合约/账户调用 issue 记分）
mpush $sbt_owner setairdrop '["mobile.rwid", true]' -p $sbt_owner
mpush $sbt_owner setairdrop '["email.rwid", true]' -p $sbt_owner
# 创作者（允许 create）
mpush $sbt_owner setcreator '["mywallet2", true]'    -p $sbt_owner

# 公证人（允许 notarize）
mpush $sbt_owner setnotary '["'"${notary_owner}"'", true]'   -p $sbt_owner




# token_id 即 nstats 主键（就是 nsymbol.id）
mpush $sbt_owner notarize '["'"${notary_owner}"'", 1]' -p $notary_owner

#向发行账号转账
mpush $sbt_owner issue '["mywallet2", {"amount":100000, "symbol":{"id":1,"pid":512}}, "add 100 to mywallet2"]' -p mywallet2

# 假设 award 给用户 alice，SBT 为 nsymbol{id:1,pid:0}，加 100 分
mpush $sbt_owner transfer '["mywallet2","ipowner.111", {"amount":100, "symbol":{"id":1,"pid":512}}, "add 100 to ipowner.111"]' -p mywallet2

 