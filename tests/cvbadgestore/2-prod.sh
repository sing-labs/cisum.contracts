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

mpush $stor_con addwhitelist '["cisum.token"]' -p $stor_con

mpush $stor_con setadmin '["flonian"]' -p $stor_con

mpush $stor_con setbadge '["'"${badge_ntoken}"'","'"${badge_ntoken}"'"]' -p $stor_con


#铜质勋章
mpush $badge_ntoken create '[
  "'"${stor_con}"'",
  10000000000,
  {"value":"4000000001"},
  "https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreihta3lzus36agk5nusdhotodzlgiciup5a4qkkah4eonp6zpxdwee",
  "'"${stor_con}"'"
]' -p $stor_con



mpush $badge_ntoken issue '[
  "'"${stor_con}"'",
  {"amount": 10000000, "symbol": {"value":"4000000001"} },
  "bootstrap batch"
]' -p $stor_con

#银质勋章
mpush $badge_ntoken create '[
  "'"${stor_con}"'",
  10000000000,
  {"value":"4000000002"},
  "https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreibymdy43u5mjjinmvsamsdhfptdofbjyqltznxr5gnh5j53f2fxtu",
  "'"${stor_con}"'"
]' -p $stor_con


mpush $badge_ntoken issue '[
  "'"${stor_con}"'",
  {"amount": 10000000, "symbol": {"value":"4000000002"}},
  "bootstrap batch"
]' -p $stor_con


#金质勋章
mpush $badge_ntoken create '[
  "'"${stor_con}"'",
  10000000000,
  {"value":"4000000003"},
  "https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreiattahrdkqcahtervgqvwq7zatdyxwhhyzzt572ftes7ffupxwws4",
  "'"${stor_con}"'"
]' -p $stor_con


mpush $badge_ntoken issue '[
  "'"${stor_con}"'",
  {"amount": 10000000, "symbol": {"value":"4000000003"}},
  "bootstrap batch"
]' -p $stor_con


#钻石勋章

mpush $badge_ntoken create '[
  "'"${stor_con}"'",
  10000000000,
  {"value":"4000000004"},
  "https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreibt6mguuph5aotscy4g72huy5numsevbb5dmcuvq3mrlgusgor4qq",
  "'"${stor_con}"'"
]' -p $stor_con


mpush $badge_ntoken issue '[
  "'"${stor_con}"'",
  {"amount": 10000000, "symbol": {"value":"4000000004"}},
  "bootstrap batch"
]' -p $stor_con










mpush badge.cvnft settokenuri '[4000000001,
"https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreigjp2lys54xk2dwfpwmhbgsdymjxzyamqg4wqrv35adfhhgdoljgi"
]' -p badge.cvnft -p badgecvstore

mpush badge.cvnft settokenuri '[
  4000000002,
  "https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreifb2vphs3zxmf25tz5dxcleu44qfnyrf5paqaytxxaxm3flzh6bdu"
]' -p badge.cvnft -p badgecvstore
mpush badge.cvnft settokenuri '[
  4000000003,
  "https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreiaiqcmpqzvauuq5mrvh4i5caeu52egsqdsmk4uzovjg75a535q3se"
]' -p badge.cvnft -p badgecvstore

mpush badge.cvnft settokenuri '[
  4000000004,
  "https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreiaxrowvsczmuuir44f2s6xvumb5qi7wjc3sv2ulu4nngan47ilqdm"
]' -p badge.cvnft -p badgecvstore









mpush badge.cvnft transfer '["badgecvstore","cvsloteuye5z",[{"amount":1,"symbol":{"value":"4000000001"}}],"test1"]' -p badgecvstore
mpush badge.cvnft transfer '["badgecvstore","cvsloteuye5z",[{"amount":1,"symbol":{"value":"4000000002"}}],"test1"]' -p badgecvstore
mpush badge.cvnft transfer '["badgecvstore","cvsloteuye5z",[{"amount":1,"symbol":{"value":"4000000003"}}],"test1"]' -p badgecvstore
mpush badge.cvnft transfer '["badgecvstore","cvsloteuye5z",[{"amount":1,"symbol":{"value":"4000000004"}}],"test1"]' -p badgecvstore