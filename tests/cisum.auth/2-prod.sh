#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

rolemanage_con=cisum.auth
mreg flon $rolemanage_con flonian
mtran flonian $rolemanage_con "100 FLON"
mset $rolemanage_con flon.auth
mcli set account permission $rolemanage_con active --add-code






rolemanage_con=cisum.auth
admin=flonian
mpush $rolemanage_con init '["'"$admin"'"]'   -p $rolemanage_con
mpush $rolemanage_con addrole '["'"$admin"'","admin","super admin"]'   -p $admin
mpush $rolemanage_con addroleperm '["'"$admin"'","admin",["banner", "content", "venues", "show", "giveTicket", "ticketCancel", "roleManage", "userManage"],""]'   -p $admin

mpush $rolemanage_con grantrole '["'"$admin"'","'"$admin"'","admin"]' -p $admin
mpush $rolemanage_con grantrole '["'"$admin"'","cisumshowman","admin"]' -p $admin


oracle=dragonmaster
mpush $rolemanage_con addallowlist '["'"$oracle"'"]'   -p $admin
mpush $rolemanage_con grantrole '["'"$admin"'","'"$oracle"'","admin"]' -p $admin




mpush $rolemanage_con delroleperm '["flonian","admin",["banner", "content", "venues", "show", "giveTicket", "ticketCancel", "roleManage", "userManage"]]'   -p flonian
mpush $rolemanage_con delroleperm '["flonian","ops",["banner", "content", "venues", "show", "giveTicket", "ticketCancel", "roleManage"]]'   -p flonian


mpush $rolemanage_con revokerole '["flonian","flonian","admin"]'   -p flonian
mpush $rolemanage_con revokerole '["flonian","cisumshowman","admin"]'   -p flonian
mpush $rolemanage_con revokerole '["flonian","dragonmaster","admin"]'   -p flonian
mpush $rolemanage_con revokerole '["flonian","x1l5s3tgwve5","admin"]'   -p flonian
mpush $rolemanage_con revokerole '["flonian","hufhq3a1mni4","admin"]'   -p flonian
mpush $rolemanage_con revokerole '["flonian","fulgwxvwfw1m","admin"]'   -p flonian
mpush $rolemanage_con revokerole '["flonian","2knqkuxyumyk","ops"]'   -p flonian


mpush $rolemanage_con delrole '["flonian","ops"]'   -p flonian
mpush $rolemanage_con delrole '["flonian","admin"]'   -p flonian
