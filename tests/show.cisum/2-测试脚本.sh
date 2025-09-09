# ===== env.sh =====
export show_con=show23.cisum      # show.cisum 合约账号
export ADMIN=myadmin               # 管理员/发行账号（issue很可能只允许管理员）
export USER1=fulgwxvwfw1m
export USER2=flontest
export GRAB_CON=grab22.cisum      # 如 issuetograb 需要的接收合约
export NFT_BANK=cvticket.nft      # 如 nasset 里需要的 NFT 合约（按你的实现替）
# 约定一组 show/ticket
export SHOW_ID_OK=20250057
export TK_VIP=21010001000010137
export TK_STD=21010001000010136




# ===== 20_issue_happy.sh =====
# 发 1 张 VIP
mpush $show_con issue '["'$USER1'", '$SHOW_ID_OK', '$TK_VIP', 1, "send to "'$USER1'" vip-1"]' -p $ADMIN
# 发 3 张 STD
mpush $show_con issue '["'$USER2'", '$SHOW_ID_OK', '$TK_STD', 3, "happy:std-3"]' -p $ADMIN



# ===== 30_issue_edges.sh =====
# 0 张（应断言 NOT_POSITIVE / invalid）
mpush $show_con issue '["'$USER1'", '$SHOW_ID_OK', '$TK_VIP', 0, "edge:zero"]' -p $ADMIN

# 超额（假设当前余量不足，应断言 INSUFFICIENT / cap reached）
mpush $show_con issue '["'$USER1'", '$SHOW_ID_OK', '$TK_VIP', 1000000000, "edge:over-cap"]' -p $ADMIN

# 过期（若 SHOW_ID_EXPIRED 的时间窗已过，应断言 EXPIRED）
# 自行准备一个已结束的 show_id：
SHOW_ID_EXPIRED=20250058
TK_VIP_EXPIRED=21010001000010139
TK_STD_EXPIRED=21010001000010138
mpush $show_con issue '["'$USER1'", '$SHOW_ID_EXPIRED', '$TK_VIP_EXPIRED', 1, "edge:expired"]' -p $ADMIN

# ===== 40_issue_authz.sh =====
mpush $show_con issue '["'$USER1'", '$SHOW_ID_OK', '$TK_VIP', 1, "unauth"]' -p $USER1

# ===== 50_tkincrease.sh =====
# 管理员给 VIP 档 +100
mpush $show_con nftissue '[
  "'$ADMIN'",
  "'$show_con'",
  {"amount":100,"symbol":{"value":'$TK_VIP'}},
  "issue:'$SHOW_ID_OK'"
]' -p $ADMIN



# 非管理员尝试（应失败）
mpush $show_con nftissue '[
  "'$ADMIN'",
  "'$show_con'",
  {"amount":100,"symbol":{"value":'$TK_VIP'}},
  "issue:'$SHOW_ID_OK'"
]' -p $USER1




# ===== 60_issuetograb.sh =====
# 增发后再发放，验证库存生效
mpush $show_con issue '["'$USER2'", '$SHOW_ID_OK', '$TK_VIP', 50, "after-increase"]' -p $ADMIN

# 管理员给 FREE 档 +100
mpush $show_con nftissue '[
  "'$ADMIN'",
  "'$show_con'",
  {"amount":100,"symbol":{"value":'$TK_STD'}},
  "issue:'$SHOW_ID_OK'"
]' -p $ADMIN



# 非管理员尝试（应失败）
mpush $show_con nftissue '[
  "'$ADMIN'",
  "'$show_con'",
  {"amount":100,"symbol":{"value":'$TK_STD'}},
  "issue:'$SHOW_ID_OK'"
]' -p $USER1

# 增发后再发放，验证库存生效
#超过库存数量
mpush $show_con issuetograb '["'$GRAB_CON'", {"amount":500,"symbol":{"value":'$TK_STD'}}, "add:2:20250057"]' -p $ADMIN
#非管理员尝试
mpush $show_con issuetograb '["'$GRAB_CON'", {"amount":100,"symbol":{"value":'$TK_STD'}}, "add:2:20250057"]' -p $USER1
#0发放
mpush $show_con issuetograb '["'$GRAB_CON'", {"amount":0,"symbol":{"value":'$TK_STD'}}, "add:2:20250057"]' -p $ADMIN
mpush $show_con issuetograb '["'$GRAB_CON'", {"amount":-100,"symbol":{"value":'$TK_STD'}}, "add:2:20250057"]' -p $ADMIN
#正常发放
mpush $show_con issuetograb '["'$GRAB_CON'", {"amount":100,"symbol":{"value":'$TK_STD'}}, "add:2:20250057"]' -p $ADMIN



# ===== 70_mix_extremes.sh =====
# 连续多次小额发放，逼近库存边界
for i in $(seq 1 25); do
  mpush $show_con issue '["'$USER1'", '$SHOW_ID_OK', '$TK_VIP', 1, "loop:'$i'"]' -p $ADMIN
done

# 先发到只剩不足1张，再试发 2 张（应失败）
# 这里给个大数以确保触发不足
mpush $show_con issue '["'$USER1'", '$SHOW_ID_OK', '$TK_VIP', 999999, "edge:almost-empty"]' -p $ADMIN
mpush $show_con issue '["'$USER1'", '$SHOW_ID_OK', '$TK_VIP', 2, "edge:not-enough-2"]' -p $ADMIN


#批量发放
mpush $show_con giftbatch '["flonian",
  20250057,
  21010001000010137,
  1,
  [
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest"
],
  "batch gift issue"
]' -p flonian


#发放超过500账号
mpush $show_con giftbatch '["flonian",
  20250057,
  21010001000010137,
  1,
  [
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest",
"myadmin","testtest","fulgwxvwfw1m","flontest"
],
  "batch gift issue"
]' -p flonian


#无效账号
mpush $show_con giftbatch '["flonian",
  20250057,
  21010001000010137,
  200,
["myadmin","testtest","fulgwxvwfw1m","flontest",
"sfgasgsags"],
  "batch gift issue"
]' -p flonian

#未授权操作者

mpush $show_con giftbatch '["gahbnbehaskk",
  20250057,
  21010001000010137,
  200,
["myadmin","testtest","fulgwxvwfw1m","flontest"],
  "batch gift issue"
]' -p gahbnbehaskk

#错误的 show/ticket

mpush $show_con giftbatch '["flonian",
  202500590,
  21010001000010137,
  200,
["myadmin","testtest","fulgwxvwfw1m","flontest"],
  "batch gift issue"
]' -p flonian


#过长 memo
LONG_MEMO=$(head -c 300 < /dev/zero | tr '\0' 'a')

mpush $show_con giftbatch '[
  "'$ADMIN'",
  '$SHOW_ID_OK',
  '$TK_VIP',
  1,
  ["'$USER1'","'$USER2'"],
  "'"$LONG_MEMO"'"
]' -p $ADMIN