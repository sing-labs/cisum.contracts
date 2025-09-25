#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

nestar_token=song.token
mreg flon $nestar_token flonian
mtran flonian $nestar_token "100 FLON"
mset $nestar_token nestar.token
mcli set account permission $nestar_token active --add-code


badgestore_contract=badgecvstore
mpush $badgestore_contract addwhitelist '["song.token"]' -p $badgestore_contract

issuer_owner=flonian
user_admin=

mpush $nestar_token init '["'"${issuer_owner}"'","'"${user_admin}"'","'"${badgestore_contract}"'"]' -p $nestar_token


# 参数：issuer, maximum_supply
mpush $nestar_token create '["'"${issuer_owner}"'", "10000000000000.0000 SONG"]' -p $nestar_token

mpush $nestar_token issue '["'"${issuer_owner}"'", "1000000000.0000 SONG", "bootstrap"]' -p $issuer_owner

# 新增规则（id=0 表示新增）
mpush $nestar_token  addbrule '[0, "15000.0000 SONG", {"value":"4299369637478511"}, true]' -p $nestar_token
mpush $nestar_token  addbrule '[0, "150000.0000 SONG", {"value":"4299369637478512"}, true]' -p $nestar_token
mpush $nestar_token  addbrule '[0, "1500000.0000 SONG", {"value":"4299369637478513"}, true]' -p $nestar_token
mpush $nestar_token  addbrule '[0, "15000000.0000 SONG", {"value":"4299369637478514"}, true]' -p $nestar_token



# mpush $nestar_token  delbrule '[0]' -p $nestar_token
# mpush $nestar_token  delbrule '[1]' -p $nestar_token
# mpush $nestar_token  delbrule '[2]' -p $nestar_token
# mpush $nestar_token  delbrule '[3]' -p $nestar_token


#要转给poe定量的nestar
#mpush $nestar_token transfer '["'"${issuer_owner}"'","cvhch4tqi1w3", "3000.0000 SONG", "bootstrap"]' -p $issuer_owner




#mpush nestar.token transfer '["gahbnbehaskk","grab.cisum","6000.0000 SONG","grab:19:a1799ae8e1ea62a20a5f56710342a235"]' -p gahbnbehaskk