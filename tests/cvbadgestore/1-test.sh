

stor_con=badgestore11
mreg flon $stor_con flonian
mtran flonian $stor_con "100 FLON"
mset $stor_con cvbadgestore
mcli set account permission $stor_con active --add-code




badge_ntoken=cvbadge.nft
mpush $badge_ntoken  addwhitelist '["badgestore11"]'   -p $badge_ntoken
mpush $stor_con addwhitelist '["nestar.token"]' -p $stor_con


#铜质勋章
mpush $badge_ntoken create '[
  "badgestore11",
  10000000000,
  {"value":"4299369637478511"},
  "ipfs://badges/silver11.json",
  "badgestore11"
]' -p badgestore11


mpush $badge_ntoken issue '[
  "badgestore11",
  {"amount": 10000000, "symbol": {"value":"4299369637478511"} },
  "bootstrap batch"
]' -p badgestore11

#银质勋章
mpush $badge_ntoken create '[
  "badgestore11",
  10000000000,
  {"value":"4299369637478512"},
  "ipfs://badges/silver12.json",
  "badgestore11"
]' -p badgestore11


mpush $badge_ntoken issue '[
  "badgestore11",
  {"amount": 10000000, "symbol": {"value":"4299369637478512"}},
  "bootstrap batch"
]' -p badgestore11


#金质勋章
mpush $badge_ntoken create '[
  "badgestore11",
  10000000000,
  {"value":"4299369637478513"},
  "ipfs://badges/silver13.json",
  "badgestore11"
]' -p badgestore11


mpush $badge_ntoken issue '[
  "badgestore11",
  {"amount": 10000000, "symbol": {"value":"4299369637478513"}},
  "bootstrap batch"
]' -p badgestore11


#钻石勋章

mpush $badge_ntoken create '[
  "badgestore11",
  10000000000,
  {"value":"4299369637478514"},
  "ipfs://badges/silver14.json",
  "badgestore11"
]' -p badgestore11


mpush $badge_ntoken issue '[
  "badgestore11",
  {"amount": 10000000, "symbol": {"value":"4299369637478514"}},
  "bootstrap batch"
]' -p badgestore11





mpush $badge_ntoken create '[
  "badgestore11",
  10000000000,
  {"value":"4299369637478514"},
  "ipfs://badges/silver14.json",
  "badgestore11"
]' -p badgestore11


mpush $badge_ntoken issue '[
  "badgestore11",
  {"amount": 10000000, "symbol": {"value":"4299369637478514"}},
  "bootstrap batch"
]' -p badgestore11



mpush $stor_con setadmin '["flonian"]' -p $stor_con

mpush $stor_con setbadge '["'"${badge_ntoken}"'","'"${badge_ntoken}"'"]' -p $stor_con





mpush badge.cvnft transfer '["badgecvstore","cvsloteuye5z",[{"amount":1,"symbol":{"value":"4299369637478516"}}],"test1"]' -p badgecvstore



mpush $badge_ntoken create '[
  "'"${stor_con}"'",
  10000000000,
  {"value":"4299369637478516"},
  "https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreibt6mguuph5aotscy4g72huy5numsevbb5dmcuvq3mrlgusgor4qq",
  "'"${stor_con}"'"
]' -p $stor_con


mpush $badge_ntoken issue '[
  "'"${stor_con}"'",
  {"amount": 10000000, "symbol": {"value":"4299369637478516"}},
  "bootstrap batch"
]' -p $stor_con





stor_con=badgecvstore
badge_ntoken=badge.cvnft
mpush $badge_ntoken create '[
  "'"${stor_con}"'",
  10000000000,
  {"nid":"5000000002"},
  "https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreianplyoyyr7qji4irgrhxfupwda37myrwl3zfecp74g4iumydf35m",
  "'"${stor_con}"'"
]' -p $stor_con


mpush $badge_ntoken issue '[
  "'"${stor_con}"'",
  {"amount": 50000, "symbol": {"nid":"5000000002"}},
  "bootstrap batch"
]' -p $stor_con






stor_con=badgecvstore
badge_ntoken=badge.cvnft
mpush $badge_ntoken create '[
  "'"${stor_con}"'",
  10000000000,
  {"nid":"5000000003"},
  "https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreibqffnhc6y4nvvsylduiezidzy6l6yum2rals7oq7g7plj7hp7gwm",
  "'"${stor_con}"'"
]' -p $stor_con


mpush $badge_ntoken issue '[
  "'"${stor_con}"'",
  {"amount": 50000, "symbol": {"nid":"5000000003"}},
  "bootstrap batch"
]' -p $stor_con

