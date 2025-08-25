poe_con=poe11.cisum
mreg flon $poe_con flonian
mtran flonian $poe_con "100 FLON"
mset $poe_con poe.cisum
mcli set account permission $poe_con active --add-code


# 给合约开 NESTAR 余额行（RAM 自付）
mpush nest15.token open '[
  "'"${poe_con}"'",
  "4,NESTAR",
  "'"${poe_con}"'"
]' -p $poe_con

# 将合约加入白名单（允许转账 NESTAR）
mpush nest15.token addwhitelist '["'"${poe_con}"'"]' -p nest15.token

# 给合约充值奖励池（示例从 flonian 转入）
mpush nest15.token transfer '[
  "nes11.issuer",
  "'"${poe_con}"'",
  "1000000.0000 NESTAR",
  "seed for PoE rewards"
]' -p nes11.issuer



# 日常签到 - 10 NESTAR
mpush $poe_con setact '[
  "signin",
  "10.0000 NESTAR",
  "日常签到奖励"
]' -p $poe_con

# 投票 - 50 NESTAR
mpush $poe_con setact '[
  "vote",
  "50.0000 NESTAR",
  "投票奖励"
]' -p $poe_con

# 发布短视频内容 - 100 NESTAR
mpush $poe_con setact '[
  "shortvideo",
  "100.0000 NESTAR",
  "发布短视频内容奖励"
]' -p $poe_con

# 邀请好友注册 - 300 NESTAR
mpush $poe_con setact '[
  "invite",
  "300.0000 NESTAR",
  "邀请好友注册奖励"
]' -p $poe_con

# 上传艺人内容 - 200 NESTAR
mpush $poe_con setact '[
  "uploadartist",
  "200.0000 NESTAR",
  "上传艺人内容奖励"
]' -p $poe_con



# 用户领取日常签到积分
mpush $poe_con claimpoints '["flonian", "signin"]' -p flonian

# 用户领取上传艺人内容积分
mpush $poe_con claimpoints '["flonian", "uploadartist"]' -p flonian