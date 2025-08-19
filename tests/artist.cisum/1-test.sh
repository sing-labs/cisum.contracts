artist_token=art14.token
mreg flon $artist_token flonian
mtran flonian $artist_token "100 FLON"
mset $artist_token artist.cisum
mcli set account permission $artist_token active --add-code



#创建艺人账号
artister_owner=artist.acc1
mreg flon $artister_owner flonian
mtran flonian $artister_owner "100 FLON"



# 创建admin
admin_owner=myadmin
mreg flon $admin_owner flonian
mtran flonian $admin_owner "100 FLON"


# 创建审核员
auditor_owner=moduser1
mreg flon $auditor_owner flonian
mtran flonian $auditor_owner "100 FLON"

 
mpush $artist_token init '["'"${admin_owner}"'",["'"${auditor_owner}"'"] ]' -p $artist_token

mpush $artist_token setadmin '["'"${admin_owner}"'"]' -p $artist_token

mpush $artist_token addauditor '["'"${auditor_owner}"'"]' -p $artist_token

mpush $artist_token delauditor '["'"${auditor_owner}"'"]' -p  $artist_token

mpush $artist_token addartist '[
  "'"${artister_owner}"'",
  "DJ Star",
  "https://img/ava.png",
  "https://img/banner.png",
  "Electronic music producer",
  "CN",
  "zh",
  "{\"twitter\":\"https://x.com/djstar\"}",
  "enabled"
]' -p $artist_token



mpush $artist_token setstatus '["'"${artister_owner}"'", "enabled", true]' -p $admin_owner