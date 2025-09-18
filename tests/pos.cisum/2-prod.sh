pos_con=pos.cisum
mreg flon $pos_con flonian
mtran flonian $pos_con "100 FLON"
mset $pos_con pos.cisum
mcli set account permission $pos_con active --add-code


#创建惩罚接收账号
mreg flon share.cisum flonian


mpush $pos_con  init '[{"sym":"8,CISUM","contract":"cisum.token"},"100.00000000 CISUM"    ]' -p $pos_con

#将合约加入到nestar白名单
mpush  nestar.token  addwhitelist '["'"${pos_con}"'"]' -p nestar.token

mpush  nestar.token  addconsumewl '["'"${pos_con}"'"]' -p nestar.token

mpush $pos_con setplan '[
  1,
  {
    "type":"term",
    "pool_type":"nestpont",
    "ir_scheme":"lad1",
    "deposit_term_days":90,
    "allow_advance_redeem":true,
    "advance_redeem_fine_rate":3000,
    "effective_from": "2025-08-01T00:00:00",
    "effective_to": "2026-11-25T00:00:00",
    "interest_token":{"sym":"4,NESTAR","contract":"nest21.token"},
    "music_reward_rate":1000
  }
]' -p $pos_con

mpush $pos_con setplan '[
  2,
  {
    "type":"term",
    "pool_type":"cisumapr",
    "ir_scheme":"lad1",
    "deposit_term_days":90,
    "allow_advance_redeem":true,
    "advance_redeem_fine_rate":3000,
    "effective_from": "2025-08-01T00:00:00",
    "effective_to": "2026-11-25T00:00:00",
   "interest_token":{"sym":"8,CISUM","contract":"cisum.token"},
    "music_reward_rate":0
  }
]' -p $pos_con




# 给计划2补充 1,000 CISUM 作为利息池
mpush cisum.token transfer '[
  "flonian",
  "'"${pos_con}"'",
  "10000.00000000 CISUM",
  "refuel:2"
]' -p flonian

mpush nestar.token transfer '[
  "nes11.issuer",
  "'"${pos_con}"'",
  "10000.0000 NESTAR",
  "refuel:1"
]' -p nes11.issuer



mpush $cisum_token transfer '[
  "flonian",
  "'"${pos_con}"'",
  "100000.00000000 MUSIC",
  "refuel"
]' -p flonian


mpush $pos_con  delplan '[1]'  -p $pos_con




mpush cisum.token transfer '[
  "flonian",
  "'"${pos_con}"'",
  "10000.00000000 CISUM",
  "deposit:2"
]' -p flonian


mpush cisum.token transfer '[
  "flonian",
  "'"${pos_con}"'",
  "10000.00000000 CISUM",
  "deposit:1"
]' -p flonian


#返还music
mpush cisum.token transfer '[
  "flonian",
  "'"${pos_con}"'",
  "1000.00000000 MUSIC",
  "return:2"
]' -p flonian


mpush $pos_con withdraw '[
  "flonian",
  "flonian",
  "2"
]' -p flonian


