rolemanage_con=rolemanage12
mreg flon $rolemanage_con flonian
mtran flonian $rolemanage_con "100 FLON"
mset $rolemanage_con rolemanage
mcli set account permission $rolemanage_con active --add-code


mpush $rolemanage_con  init '["flonian"]'   -p $rolemanage_con
mpush $rolemanage_con  setadmin '["flonian"]'   -p flonian


mpush $rolemanage_con addoracle '["myadmin"]' -p flonian



mpush $rolemanage_con  setrole '["admin","超级管理员"]'   -p flonian
mpush $rolemanage_con setrole '["oracle","数据喂价员"]' -p flonian
mpush $rolemanage_con setrole '["editor","编辑员"]' -p flonian

mpush $rolemanage_con  delrole '["admin"]'   -p flonian


mpush $rolemanage_con grantrole '["show24.cisum","flonian","user1","oracle"]' -p flonian


mpush rolemanage12 revokerole '["show24.cisum","flonian","user1","oracle"]' -p flonian