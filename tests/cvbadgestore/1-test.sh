

stor_con=badgestore11
mreg flon $stor_con flonian
mtran flonian $stor_con "100 FLON"
mset $stor_con cvbadgestore
mcli set account permission $stor_con active --add-code




badge_ntoken=cvbadge.nft
 


mpush $stor_con setadmin '["flonian"]' -p $stor_con

mpush $stor_con setbadge '["'"${badge_ntoken}"'","'"${badge_ntoken}"'"]' -p $stor_con


mpush $stor_con addwhitelist '["nest15.token"]' -p $stor_con




