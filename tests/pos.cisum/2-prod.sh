#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

pos_con=pos.cisum
mreg flon $pos_con flonian
mtran flonian $pos_con "100 FLON"
mset $pos_con pos.cisum
mcli set account permission $pos_con active --add-code


#创建惩罚接收账号
mreg flon share.cisum flonian


mpush $pos_con  init '[{"sym":"8,SING","contract":"sing.token"},"100.00000000 SING"    ]' -p $pos_con

#将合约加入到nestar白名单
mpush  song.token  addwhitelist '["'"${pos_con}"'"]' -p song.token

mpush  song.token  addconsumewl '["'"${pos_con}"'"]' -p song.token

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
    "interest_token":{"sym":"4,SONG","contract":"song.token"},
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
   "interest_token":{"sym":"8,SING","contract":"sing.token"},
    "music_reward_rate":0
  }
]' -p $pos_con




# 给计划2补充 1,000 CISUM 作为利息池
mpush sing.token transfer '[
  "flonian",
  "'"${pos_con}"'",
  "10000.00000000 SING",
  "refuel:2"
]' -p flonian

mpush song.token transfer '[
  "nes11.issuer",
  "'"${pos_con}"'",
  "10000.0000 SONG",
  "refuel:1"
]' -p nes11.issuer



mpush sing.token transfer '[
  "flonian",
  "'"${pos_con}"'",
  "100000.00000000 MUSIC",
  "refuel"
]' -p flonian


mpush $pos_con  delplan '[1]'  -p $pos_con




mpush sing.token transfer '[
  "flonian",
  "'"${pos_con}"'",
  "10000.00000000 SING",
  "deposit:2"
]' -p flonian


mpush sing.token transfer '[
  "flonian",
  "'"${pos_con}"'",
  "10000.00000000 SING",
  "deposit:1"
]' -p flonian


#返还music
mpush sing.token transfer '[
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


