#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

cisum_token=cisum.token
mreg flon $cisum_token flonian
mtran flonian $cisum_token "100 FLON"
mset $cisum_token cisum.token
mcli set account permission $cisum_token active --add-code


badgestore_contract=badgecvstore
mpush $badgestore_contract addwhitelist '["cisum.token"]' -p $badgestore_contract

issuer_owner=flonian
user_admin=flonian

mpush $cisum_token init '["'"${issuer_owner}"'","'"${user_admin}"'","'"${badgestore_contract}"'"]' -p $cisum_token


# 参数：issuer, maximum_supply
mpush $cisum_token create '["'"${issuer_owner}"'", "10000000000000.0000 CISUM"]' -p $cisum_token

mpush $cisum_token issue '["'"${issuer_owner}"'", "1000000000.0000 CISUM", "bootstrap"]' -p $issuer_owner
#转入到poe合约
mpush $cisum_token transfer '["'"${issuer_owner}"'","poe.cisum", "200000000.0000 CISUM", "bootstrap"]' -p $issuer_owner

# 新增规则（id=0 表示新增）
mpush $cisum_token  addbrule '[0, "15000.0000 CISUM", {"value":"4299369637478511"}, true]' -p $cisum_token
mpush $cisum_token  addbrule '[0, "150000.0000 CISUM", {"value":"4299369637478512"}, true]' -p $cisum_token
mpush $cisum_token  addbrule '[0, "1500000.0000 CISUM", {"value":"4299369637478513"}, true]' -p $cisum_token
mpush $cisum_token  addbrule '[0, "15000000.0000 CISUM", {"value":"4299369637478514"}, true]' -p $cisum_token



# mpush $nestar_token  delbrule '[0]' -p $nestar_token
# mpush $nestar_token  delbrule '[1]' -p $nestar_token
# mpush $nestar_token  delbrule '[2]' -p $nestar_token
# mpush $nestar_token  delbrule '[3]' -p $nestar_token


#要转给poe定量的nestar
#mpush $cisum_token transfer '["'"${issuer_owner}"'","cvug42btfnsi", "3000.0000 CISUM", "bootstrap"]' -p $issuer_owner




#mpush nestar.token transfer '["gahbnbehaskk","grab.cisum","6000.0000 SONG","grab:19:a1799ae8e1ea62a20a5f56710342a235"]' -p gahbnbehaskk


mpush cisum.token transfer '["flonian","cv2qvpg53rhk", "400.0000 CISUM", "bootstrap"]' -p flonian


