
badge_ntoken=badge.cvnft
mreg flon $badge_ntoken flonian
mtran flonian $badge_ntoken "100 FLON"
mset $badge_ntoken cvticket.nft
mcli set account permission $badge_ntoken active --add-code



