ops_con=cisum.vault
ops_admin=flonian

mreg flon $ops_con flonian
mtran flonian $ops_con "100 FLON"
mset $ops_con cisum.vault
mcli set account permission $ops_con active --add-code

mpush $ops_con init '["'"${ops_admin}"'"]' -p $ops_con

mpush cisum.token addconsumewl '["'"${ops_con}"'"]' -p cisum.token
