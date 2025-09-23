#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

poe_con=poe.cisum
mreg flon $poe_con flonian
mtran flonian $poe_con "100 FLON"
mset $poe_con poe.cisum
mcli set account permission $poe_con active --add-code

nestar_token=nestar.token
# 给合约开 NESTAR 余额行（RAM 自付）
mpush $nestar_token open '[
  "'"${poe_con}"'",
  "4,SONG",
  "'"${poe_con}"'"
]' -p $poe_con

# 将合约加入白名单（允许转账 NESTAR）
mpush $nestar_token addwhitelist '["'"${poe_con}"'"]' -p $nestar_token


# 日常签到 - 10 NESTAR
mpush $poe_con addrewardact '[
  "signin",
  "10.0000 SONG",
  "日常签到奖励"
]' -p $poe_con

# 投票 - 50 NESTAR
mpush $poe_con addrewardact '[
  "vote",
  "50.0000 SONG",
  "投票奖励"
]' -p $poe_con

# 发布短视频内容 - 100 NESTAR
mpush $poe_con addrewardact '[
  "shortvideo",
  "100.0000 SONG",
  "发布短视频内容奖励"
]' -p $poe_con

# 邀请好友注册 - 300 NESTAR
mpush $poe_con addrewardact '[
  "invite",
  "300.0000 SONG",
  "邀请好友注册奖励"
]' -p $poe_con

# 上传艺人内容 - 200 NESTAR
mpush $poe_con addrewardact '[
  "uploadartist",
  "200.0000 SONG",
  "上传艺人内容奖励"
]' -p $poe_con

mpush $poe_con addoracle '[
  "myadmin"
]' -p $poe_con
