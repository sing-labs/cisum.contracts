#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

rolemanage_con=flon.auth
mreg flon $rolemanage_con flonian
mtran flonian $rolemanage_con "100 FLON"
mset $rolemanage_con flon.auth
mcli set account permission $rolemanage_con active --add-code

admin=flonian
mpush $rolemanage_con  init '["'"$admin"'"]'   -p $rolemanage_con
mpush $rolemanage_con  addrole '["admin","超级管理员"]'   -p $admin
mpush $rolemanage_con  addrole '["oracle",""]' -p $admin
mpush $rolemanage_con  setrole '["showadmin","演唱会管理员"]'   -p $admin
mpush $rolemanage_con  setrole '["platformadmin","平台管理员"]'   -p $admin
mpush $rolemanage_con  setrole '["R_REDEMPTION_EXECUTE","核销用户"]'   -p $admin
mpush $rolemanage_con  setrole '["R_CREATE_ROLE","角色权限修改"]'   -p $admin

mpush $rolemanage_con grantrole '["flon.auth","flonian","flonian","R_CREATE_ROLE"]' -p $admin
mpush $rolemanage_con grantrole '["grab.cisum","'"$admin"'","'"$admin"'","admin"]' -p $admin
oracle=
mpush $rolemanage_con  addallowlist '["'"$oracle"'"]'   -p $admin
mpush $rolemanage_con grantrole '["grab.cisum","'"$admin"'","'"$oracle"'","admin"]' -p $admin
mpush $rolemanage_con grantrole '["show.cisum","'"$admin"'","'"$admin"'","admin"]' -p $admin
mpush $rolemanage_con grantrole '["show.cisum","'"$admin"'","'"$oracle"'","showadmin"]' -p $admin
mpush $rolemanage_con grantrole '["show.cisum","'"$admin"'","cisumshowman","admin"]' -p $admin

