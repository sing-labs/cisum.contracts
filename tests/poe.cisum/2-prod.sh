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


mpush poe.cisum addrewardact '[
  "promoteshow",
  "100.0000 CISUM",
  "Concert Promotion Rewards"
]' -p poe.cisum

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





mpush poe.cisum addrewardact '[
  "tiktokfollow",
  "5.0000 CISUM",
  "Tiktok user follows Cisumverse artists"
]' -p poe.cisum

mpush poe.cisum addrewardact '[
  "tiktokpost",
  "15.0000 CISUM",
  "Tiktok user posts Cisumverse articles"
]' -p poe.cisum

mpush poe.cisum addrewardact '[
  "tiktokrepost",
  "10.0000 CISUM",
  "Tiktok user reposts Cisumverse articles"
]' -p poe.cisum

mpush poe.cisum addrewardact '[
  "tiktoklike",
  "1.0000 CISUM",
  "Tiktok user likes Cisumverse posts"
]' -p poe.cisum

mpush poe.cisum addrewardact '[
  "tiktokreply",
  "3.0000 CISUM",
  "Tiktok user replies Cisumverse posts"
]' -p poe.cisum



mpush poe.cisum addrewardact '[
  "setnickname",
  "20.0000 CISUM",
  "set your nickname 1st time"
]' -p poe.cisum


mpush poe.cisum addrewardact '[
  "setavatar",
  "20.0000 CISUM",
  "set your avatar 1st time"
]' -p poe.cisum




mpush poe.cisum addrewardact '[
  "xcomfollow",
  "5.0000 CISUM",
  "Twitter/X user follows Cisumverse artists"
]' -p poe.cisum

mpush poe.cisum addrewardact '[
  "xcompost",
  "15.0000 CISUM",
  "Twitter/X user posts Cisumverse articles"
]' -p poe.cisum

mpush poe.cisum addrewardact '[
  "xcomrepost",
  "10.0000 CISUM",
  "Twitter/X user reposts Cisumverse articles"
]' -p poe.cisum

mpush poe.cisum addrewardact '[
  "xcomlike",
  "1.0000 CISUM",
  "Twitter/X user likes Cisumverse posts"
]' -p poe.cisum

mpush poe.cisum addrewardact '[
  "xcomreply",
  "3.0000 CISUM",
  "Twitter/X user replies Cisumverse posts"
]' -p poe.cisum






poe_con=poe.cisum
# songa - 40 CISUM
mpush $poe_con addrewardact '[
  "songa",
  "40.0000 CISUM",
  "Create an A grade song  Publish one song with grade A"
]' -p $poe_con

# songb - 20 CISUM
mpush $poe_con addrewardact '[
  "songb",
  "20.0000 CISUM",
  "Create a B grade song  Publish one song with grade B or above"
]' -p $poe_con


# tophundred - 50 CISUM
mpush $poe_con addrewardact '[
  "tophundred",
  "50.0000 CISUM",
  "Enter top 100    Any published song enters the top 100 chart"
]' -p $poe_con


# invite - 200 CISUM
mpush $poe_con addrewardact '[
  "invite",
  "200.0000 CISUM",
  "nvite a friend    Invite one friend to register"
]' -p $poe_con


# postday - 15 CISUM
mpush $poe_con addrewardact '[
  "postday",
  "15.0000 CISUM",
  "Publish one post    Publish one song to community"
]' -p $poe_con


# replyday - 3 CISUM
mpush $poe_con addrewardact '[
  "replyday",
  "3.0000 CISUM",
  "Reply to comments   Reply to comments"
]' -p $poe_con


# likeday - 1 CISUM
mpush $poe_con addrewardact '[
  "likeday",
  "1.0000 CISUM",
  "Like a post or song   Complete one like action"
]' -p $poe_con

# shareday - 10 CISUM
mpush $poe_con addrewardact '[
  "shareday",
  "10.0000 CISUM",
  "Share a post     Share one published song"
]' -p $poe_con

# listenday - 2 CISUM
mpush $poe_con addrewardact '[
  "listenday",
  "2.0000 CISUM",
  "Listen for two minutes  Accumulate 120 seconds of listening time"
]' -p $poe_con

# signday - 5 CISUM
mpush $poe_con addrewardact '[
  "signday",
  "5.0000 CISUM",
  " Daily sign-in     Complete one sign-in today"
]' -p $poe_con


# weeksignin - 5 CISUM
mpush $poe_con addrewardact '[
  "weeksignin",
  "5.0000 CISUM",
  "7-day sign-in bonus   Extra reward for every 7 consecutive sign-ins"
]' -p $poe_con
