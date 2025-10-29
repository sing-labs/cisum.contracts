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



# 首次关注艺人 - 5 CISUM
mpush $poe_con addrewardact '[
  "followartist",
  "5.0000 CISUM",
  "首次关注艺人"
]' -p $poe_con

# 发布贴文 - 15 CISUM
mpush $poe_con addrewardact '[
  "postarticle",
  "15.0000 CISUM",
  "发布贴文"
]' -p $poe_con

#点赞艺人动态 - 1 CISUM
mpush $poe_con addrewardact '[
  "likeartist",
  "1.0000 CISUM",
  "点赞艺人动态"
]' -p $poe_con

# 回复艺人动态 - 3 CISUM
mpush $poe_con addrewardact '[
  "replyartist",
  "3.0000 CISUM",
  "回复艺人动态"
]' -p $poe_con

# 转发艺人动态 - 10 CISUM
mpush $poe_con addrewardact '[
  "repostartist",
  "10.0000 CISUM",
  "转发艺人动态"
]' -p $poe_con

# 聆听艺人音乐 - 2 CISUM
mpush $poe_con addrewardact '[
  "playmusic",
  "2.0000 CISUM",
  "聆听艺人音乐"
]' -p $poe_con

#  每日签到 - 5 CISUM
mpush $poe_con addrewardact '[
  "sigindaily",
  "5.0000 CISUM",
  "每日签到"
]' -p $poe_con

# 7日连续签到 - 5 CISUM
mpush $poe_con addrewardact '[
  "siginaweek",
  "5.0000 CISUM",
  "7日连续签到"
]' -p $poe_con





mpush  $poe_con addoperator '["cisum.admin"]' -p $poe_con