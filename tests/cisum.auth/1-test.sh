rolemanage_con=flon.auth111
mreg flon $rolemanage_con flonian
mtran flonian $rolemanage_con "100 FLON"
mset $rolemanage_con flon.auth
mcli set account permission $rolemanage_con active --add-code


mpush $rolemanage_con  init '["flonian"]'   -p $rolemanage_con
mpush $rolemanage_con  setadmin '["flonian"]'   -p flonian


mpush $rolemanage_con addoracle '["myadmin"]' -p flonian
mpush $rolemanage_con addoracle '["show24.cisum"]' -p flonian
mpush $rolemanage_con addoracle '["grab23.cisum"]' -p flonian

mpush $rolemanage_con  setrole '["admin","超级管理员"]'   -p flonian
mpush $rolemanage_con setrole '["oracle","数据喂价员"]' -p flonian
mpush $rolemanage_con setrole '["editor","编辑员"]' -p flonian
mpush $rolemanage_con  setrole '["showadmin","演唱会管理员"]'   -p flonian
mpush $rolemanage_con  setrole '["platformadmin","平台管理员"]'   -p flonian
mpush $rolemanage_con  setrole '["R_REDEMPTION_EXECUTE","核销用户"]'   -p flonian


mpush $rolemanage_con  setrole '["admin","超级管理员"]'   -p flonian
mpush $rolemanage_con  delrole '["showadmin1"]'   -p flonian


mpush $rolemanage_con grantrole '["show24.cisum","flonian","flonian","admin"]' -p flonian
mpush $rolemanage_con grantrole '["show24.cisum","flonian","myadmin","showadmin"]' -p flonian
mpush $rolemanage_con grantrole '["show24.cisum","flonian","ops21.cisum","showadmin"]' -p flonian


mpush $rolemanage_con grantrole '["show24.cisum","flonian","myadmin","R_REDEMPTION_EXECUTE"]' -p flonian
mpush $rolemanage_con grantrole '["show24.cisum","flonian","cvplqwcdfvzj","R_REDEMPTION_EXECUTE"]' -p flonian

mpush $rolemanage_con grantrole '["flon.auth111","flonian","flonian","R_CREATE_ROLE"]' -p flonian

mpush $rolemanage_con revokerole '["show24.cisum","flonian","user1","oracle"]' -p flonian

mpush $rolemanage_con checkrole '["myadmin","show24.cisum","user1","admin"]' -p myadmin




mpush $rolemanage_con revokerole '["show24.cisum","flonian","ops21.cisum","show_admin"]' -p flonian



mpush $rolemanage_con addroleperm '["flonian","admin","tests","tests"]'   -p flonian


mpush $rolemanage_con delroleperm '["flonian","admin","test"]'   -p flonian


rolemanage_con=flon.auth
mpush $rolemanage_con grantrole '["flon.auth","flonian","pxuqby4g5ogz","admin"]' -p flonian



mpush $rolemanage_con addroleperm '[
  "flonian",
  "管理员",
  ["create","view","delete"],
  "批量添加权限测试"
]' -p flonian


mpush $rolemanage_con delroleperm '[
  "flonian",
  "管理员",
  ["tests","view","delete"],
  "批量删除权限测试"
]' -p flonian




mpush $rolemanage_con  addrole '["admin","超级管理员1111"]'   -p flonian

mpush $rolemanage_con  addrole '["超级管理员1111","超级管理员1111"]'   -p flonian
mpush $rolemanage_con  delrole '["admin"]'   -p flonian


mpush $rolemanage_con  setrole '["oracle","数据喂价员"]'   -p flonian
mpush $rolemanage_con  delrole '["oracle"]'   -p flonian

mpush $rolemanage_con  setrole '["editor","编辑员"]'   -p flonian
mpush $rolemanage_con  delrole '["editor"]'   -p flonian


mpush $rolemanage_con  setrole '["showadmin","演唱会管理员"]'   -p flonian
mpush $rolemanage_con  delrole '["showadmin"]'   -p flonian

mpush $rolemanage_con  setrole '["platformadmin","平台管理员"]'   -p flonian
mpush $rolemanage_con  delrole '["platformadmin"]'   -p flonian

mpush $rolemanage_con  setrole '["R_REDEMPTION_EXECUTE","核销用户"]'   -p flonian
mpush $rolemanage_con  delrole '["R_REDEMPTION_EXECUTE"]'   -p flonian

mpush $rolemanage_con  setrole '["R_CREATE_ROLE","角色权限修改"]'   -p flonian
mpush $rolemanage_con  delrole '["R_CREATE_ROLE"]'   -p flonian


mpush $rolemanage_con  addallowlist '["aaaaaaaaaaa2"]'   -p flonian

mpush $rolemanage_con grantrole '["grab.cisum","flonian","myadmin","admin"]' -p flonian





mpush $rolemanage_con  clearstate '[]'   -p flonian









mpush $rolemanage_con grantrole '["flonian","flonian","admin"]' -p flonian


rolemanage_con=flon.auth
mpush $rolemanage_con addroleperm '["flonian","admin",["show"],"show权限"]' -p flonian



mpush $rolemanage_con grantrole '["flonian","fulgwxvwfw1m","admin"]' -p flonian




mpush $rolemanage_con checkrole '["flonian","cisumshowman","show"]' -p flonian





mpush $rolemanage_con addroleperm '["flonian","admin",["banner", "content", "venues", "show", "giveTicket", "ticketCancel", "roleManage", "userManage"],""]'   -p flonian



mpush $rolemanage_con delroleperm '["flonian","admin",["create", "delete", "view"]]'   -p flonian



mpush cisum.auth addroleperm '["flonian","admin",["vlogStatisticsMgmt"],"vlogStatisticsMgmt"]'   -p flonian

mpush cisum.auth addroleperm '["flonian","admin",["appStatisticsMgmt"],"appStatisticsMgmt"]'   -p flonian