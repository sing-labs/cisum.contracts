#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

rolemanage_con=flon.auth
mreg flon $rolemanage_con flonian
mtran flonian $rolemanage_con "100 FLON"
mset $rolemanage_con flon.auth
mcli set account permission $rolemanage_con active --add-code

admin=flonian
mpush $rolemanage_con init '["'"$admin"'"]'   -p $rolemanage_con
mpush $rolemanage_con addrole '["'"$admin"'","admin","super man"]'   -p $admin
mpush $rolemanage_con addroleperm '["'"$admin"'","admin",["banner", "content", "venues", "show", "giveTicket", "ticketCancel", "roleManage", "userManage"],""]'   -p $admin

mpush $rolemanage_con grantrole '["'"$admin"'","'"$admin"'","admin"]' -p $admin
mpush $rolemanage_con grantrole '["'"$admin"'","cisumshowman","admin"]' -p $admin

oracle=dragonmaster
mpush $rolemanage_con addallowlist '["'"$oracle"'"]'   -p $admin
mpush $rolemanage_con grantrole '["'"$admin"'","'"$oracle"'","admin"]' -p $admin







mpush $rolemanage_con addroleperm '["'"$admin"'","admin",["artist"],""]'   -p $admin
