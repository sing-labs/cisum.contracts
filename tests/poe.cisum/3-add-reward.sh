# 投票 - 50 CISUM
mpush $poe_con addrewardact '[
  "vote",
  "50.0000 CISUM",
  "Earn points by voting"
]' -p $poe_con

# 发布短视频内容 - 100 CISUM
mpush $poe_con addrewardact '[
  "shortvideo",
  "100.0000 CISUM",
  "Earn points by posting short video contents"
]' -p $poe_con

# 邀请好友注册 - 200 CISUM
mpush $poe_con addrewardact '[
  "invite",
  "200.0000 CISUM",
  "Earn points by inviting friends to register"
]' -p $poe_con

# 上传艺人内容 - 200 CISUM
mpush $poe_con addrewardact '[
  "uploadartist",
  "200.0000 CISUM",
  "Earn points by uploading artists contents"
]' -p $poe_con


# 首次关注艺人 - 5 CISUM
mpush $poe_con addrewardact '[
  "followartist",
  "5.0000 CISUM",
  "Earn points by following artists"
]' -p $poe_con

# 发布贴文 - 15 CISUM
mpush $poe_con addrewardact '[
  "postarticle",
  "15.0000 CISUM",
  "Earn points by posting articles"
]' -p $poe_con

#点赞艺人动态 - 1 CISUM
mpush $poe_con addrewardact '[
  "likeartist",
  "1.0000 CISUM",
  "Earn points by liking artists events"
]' -p $poe_con

# 回复艺人动态 - 3 CISUM
mpush $poe_con addrewardact '[
  "replyartist",
  "3.0000 CISUM",
  "Earn points by replying artists events"
]' -p $poe_con

# 转发艺人动态 - 10 CISUM
mpush $poe_con addrewardact '[
  "repostartist",
  "10.0000 CISUM",
  "Earn points by reposting artists events"
]' -p $poe_con

# 聆听艺人音乐 - 2 CISUM
mpush $poe_con addrewardact '[
  "playmusic",
  "2.0000 CISUM",
  "Earn points by playing music/songs"
]' -p $poe_con

#  每日签到 - 5 CISUM
mpush $poe_con addrewardact '[
  "sigindaily",
  "5.0000 CISUM",
  "Earn points by daily sign-in"
]' -p $poe_con

# 7日连续签到 - 5 CISUM
mpush $poe_con addrewardact '[
  "siginaweek",
  "5.0000 CISUM",
  "Earn points by consecutive sign-in for 7 days"
]' -p $poe_con