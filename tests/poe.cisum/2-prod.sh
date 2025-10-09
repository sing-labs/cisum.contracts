#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

poe_con=poe.cisum
mreg flon $poe_con flonian
mtran flonian $poe_con "100 FLON"
mset $poe_con poe.cisum
mcli set account permission $poe_con active --add-code

cisum_token=cisum.token

# 将合约加入白名单（允许转账 CISUM）
mpush $cisum_token addwhitelist '["'"${poe_con}"'"]' -p $cisum_token


# 日常签到 - 10 CISUM
mpush $poe_con addrewardact '[
  "signin",
  "10.0000 CISUM",
  "日常签到奖励"
]' -p $poe_con

# 投票 - 50 CISUM
mpush $poe_con addrewardact '[
  "vote",
  "50.0000 CISUM",
  "投票奖励"
]' -p $poe_con

# 发布短视频内容 - 100 CISUM
mpush $poe_con addrewardact '[
  "shortvideo",
  "100.0000 CISUM",
  "发布短视频内容奖励"
]' -p $poe_con

# 邀请好友注册 - 200 CISUM
mpush $poe_con addrewardact '[
  "invite",
  "200.0000 CISUM",
  "邀请好友注册奖励"
]' -p $poe_con

# 上传艺人内容 - 200 CISUM
mpush $poe_con addrewardact '[
  "uploadartist",
  "200.0000 CISUM",
  "上传艺人内容奖励"
]' -p $poe_con

operator=
mpush $poe_con addoperator '[
  "'"${operator}"'"
]' -p $poe_con



