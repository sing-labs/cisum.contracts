#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

rolemanage_con=cisum.auth
mreg flon $rolemanage_con flonian
mtran flonian $rolemanage_con "100 FLON"
mset $rolemanage_con flon.auth
mcli set account permission $rolemanage_con active --add-code

admin=flonian
mpush $rolemanage_con init '["'"$admin"'"]'   -p $rolemanage_con
mpush $rolemanage_con addrole '["'"$admin"'","admin","super admin"]'   -p $admin
mpush $rolemanage_con addroleperm '["'"$admin"'","admin",["banner", "content", "venues", "show", "giveTicket", "ticketCancel", "roleManage", "userManage"],""]'   -p $admin

mpush $rolemanage_con grantrole '["'"$admin"'","'"$admin"'","admin"]' -p $admin
mpush $rolemanage_con grantrole '["'"$admin"'","cisumshowman","admin"]' -p $admin

oracle=
mpush $rolemanage_con addallowlist '["'"$oracle"'"]'   -p $admin
mpush $rolemanage_con grantrole '["'"$admin"'","'"$oracle"'","admin"]' -p $admin





oracle=dragonmaster
mpush cisum.auth addallowlist '["'"$oracle"'"]'   -p flonian
mpush cisum.auth grantrole '["flonian","'"$oracle"'","admin"]' -p flonian


mpush cisum.auth addrole '["flonian","运营",""]'   -p flonian
mpush cisum.auth addroleperm '["flonian","运营",["banner", "content", "venues", "show", "giveTicket", "ticketCancel", "roleManage"],""]'   -p flonian



mpush cisum.auth grantrole '["flonian","cisumshowman","admin"]' -p flonian
mpush cisum.auth grantrole '["flonian","x1l5s3tgwve5","admin"]' -p flonian
mpush cisum.auth grantrole '["flonian","hufhq3a1mni4","admin"]' -p flonian
mpush cisum.auth grantrole '["flonian","hufdm4lwixtp","运营"]' -p flonian

