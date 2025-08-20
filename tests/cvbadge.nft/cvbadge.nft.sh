
badge_ntoken=cvbadge.nft
mreg flon $badge_ntoken flonian
mtran flonian $badge_ntoken "100 FLON"
mset $badge_ntoken cvbadge.nft
mcli set account permission $badge_ntoken active --add-code


mcli set account permission cvbadge1.nft active \
'{
  "threshold": 1,
  "keys": [],
  "accounts": [
    { "permission": { "actor": "badgestore11", "permission": "flon.code" }, "weight": 1 },
    { "permission": { "actor": "cvbadge1.nft",  "permission": "flon.code"   }, "weight": 1 },
    { "permission": { "actor": "flonian",     "permission": "active"      }, "weight": 1 }
  ],
  "waits": []
}' owner -p cvbadge1.nft@own





mpush $badge_ntoken  addwhitelist '["cvbadge.nft"]'   -p $badge_ntoken


#铜质勋章
mpush $badge_ntoken create '[
  "'"${badge_ntoken}"'",
  10000000000,
  {"id": 101, "pid": 001001025},
  "ipfs://badges/silver.json",
  "'"${badge_ntoken}"'"
]' -p $badge_ntoken


mpush $badge_ntoken issue '[
  "'"${badge_ntoken}"'",
  {"amount": 10000000, "symbol": {"id": 101, "pid": 001001025}},
  "bootstrap batch"
]' -p $badge_ntoken

#银质勋章
mpush $badge_ntoken create '[
  "'"${badge_ntoken}"'",
  10000000000,
  {"id": 102, "pid": 001001025},
  "ipfs://badges/silver1.json",
  "'"${badge_ntoken}"'"
]' -p $badge_ntoken


mpush $badge_ntoken issue '[
  "'"${badge_ntoken}"'",
  {"amount": 10000000, "symbol": {"id": 102, "pid": 001001025}},
  "bootstrap batch"
]' -p $badge_ntoken


#金质勋章
mpush $badge_ntoken create '[
  "'"${badge_ntoken}"'",
  10000000000,
  {"id": 103, "pid": 001001025},
  "ipfs://badges/silver2.json",
  "'"${badge_ntoken}"'"
]' -p $badge_ntoken


mpush $badge_ntoken issue '[
  "'"${badge_ntoken}"'",
  {"amount": 10000000, "symbol": {"id": 103, "pid": 001001025}},
  "bootstrap batch"
]' -p $badge_ntoken


#钻石勋章

mpush $badge_ntoken create '[
  "'"${badge_ntoken}"'",
  10000000000,
  {"id": 104, "pid": 001001025},
  "ipfs://badges/silver3.json",
  "'"${badge_ntoken}"'"
]' -p $badge_ntoken


mpush $badge_ntoken issue '[
  "'"${badge_ntoken}"'",
  {"amount": 10000000, "symbol": {"id": 104, "pid": 001001025}},
  "bootstrap batch"
]' -p $badge_ntoken

