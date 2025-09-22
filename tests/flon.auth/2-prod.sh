rolemanage_con=flon.auth
mreg flon $rolemanage_con flonian
mtran flonian $rolemanage_con "100 FLON"
mset $rolemanage_con flon.auth
mcli set account permission $rolemanage_con active --add-code


mpush $rolemanage_con  init '["flonian"]'   -p $rolemanage_con
mpush $rolemanage_con  setadmin '["flonian"]'   -p flonian


mpush $rolemanage_con  setrole '["admin","超级管理员"]'   -p flonian
mpush $rolemanage_con   setrole '["oracle","数据喂价员"]' -p flonian
mpush $rolemanage_con   setrole '["editor","编辑员"]' -p flonian
mpush $rolemanage_con  setrole '["showadmin","演唱会管理员"]'   -p flonian
mpush $rolemanage_con  setrole '["platformadmin","平台管理员"]'   -p flonian
mpush $rolemanage_con  setrole '["R_REDEMPTION_EXECUTE","核销用户"]'   -p flonian
mpush $rolemanage_con  setrole '["R_CREATE_ROLE","角色权限修改"]'   -p flonian




mpush $rolemanage_con grantrole '["show.cisum","flonian","flonian","admin"]' -p flonian
mpush $rolemanage_con grantrole '["show.cisum","flonian","myadmin","showadmin"]' -p flonian
mpush $rolemanage_con grantrole '["show.cisum","flonian","cisumshowman","showadmin"]' -p flonian
mpush $rolemanage_con grantrole '["show.cisum","flonian","myadmin","R_REDEMPTION_EXECUTE"]' -p flonian
mpush $rolemanage_con grantrole '["show.cisum","flonian","cvplqwcdfvzj","R_REDEMPTION_EXECUTE"]' -p flonian
mpush $rolemanage_con grantrole '["flon.auth","flonian","flonian","R_CREATE_ROLE"]' -p flonian
mpush $rolemanage_con grantrole '["grab.cisum","flonian","flonian","admin"]' -p flonian



#设置白名单（可以操作role、userrole、roleperm）
#mpush $rolemanage_con  addallowlist '["fulgwxvwfw1m"]'   -p flonian



#grab 有admin 、oracle权限 ，如果需要修改功能要加这个权限
# mpush $rolemanage_con grantrole '["grab.cisum","flonian","pxuqby4g5ogz","admin"]' -p flonian