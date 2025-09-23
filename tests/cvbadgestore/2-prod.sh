#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc


stor_con=badgecvstore
mreg flon $stor_con flonian
mtran flonian $stor_con "100 FLON"
mset $stor_con cvbadgestore
mcli set account permission $stor_con active --add-code




badge_ntoken=badge.cvnft
mpush $badge_ntoken  addwhitelist '["'"${stor_con}"'"]'   -p $badge_ntoken

mpush $stor_con addwhitelist '["nestar.token"]' -p $stor_con
mpush $stor_con setadmin '["flonian"]' -p $stor_con

mpush $stor_con setbadge '["'"${badge_ntoken}"'","'"${badge_ntoken}"'"]' -p $stor_con


#铜质勋章
mpush $badge_ntoken create '[
  "'"${stor_con}"'",
  10000000000,
  {"value":"4299369637478511"},
  "ipfs://badges/silver21.json",
  "'"${stor_con}"'"
]' -p $stor_con


mpush $badge_ntoken issue '[
  "'"${stor_con}"'",
  {"amount": 10000000, "symbol": {"value":"4299369637478511"} },
  "bootstrap batch"
]' -p $stor_con

#银质勋章
mpush $badge_ntoken create '[
  "'"${stor_con}"'",
  10000000000,
  {"value":"4299369637478512"},
  "ipfs://badges/silver12.json",
  "'"${stor_con}"'"
]' -p $stor_con


mpush $badge_ntoken issue '[
  "'"${stor_con}"'",
  {"amount": 10000000, "symbol": {"value":"4299369637478512"}},
  "bootstrap batch"
]' -p $stor_con


#金质勋章
mpush $badge_ntoken create '[
  "'"${stor_con}"'",
  10000000000,
  {"value":"4299369637478513"},
  "ipfs://badges/silver13.json",
  "'"${stor_con}"'"
]' -p $stor_con


mpush $badge_ntoken issue '[
  "'"${stor_con}"'",
  {"amount": 10000000, "symbol": {"value":"4299369637478513"}},
  "bootstrap batch"
]' -p $stor_con


#钻石勋章

mpush $badge_ntoken create '[
  "'"${stor_con}"'",
  10000000000,
  {"value":"4299369637478514"},
  "ipfs://badges/silver14.json",
  "'"${stor_con}"'"
]' -p $stor_con


mpush $badge_ntoken issue '[
  "'"${stor_con}"'",
  {"amount": 10000000, "symbol": {"value":"4299369637478514"}},
  "bootstrap batch"
]' -p $stor_con






