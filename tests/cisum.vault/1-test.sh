#!/bin/bash
set -e

ops_con=cisum.vault
ops_admin=flonian
user=gahbnbehaskk

mreg flon $ops_con flonian
mtran flonian $ops_con "100 FLON"
mset $ops_con cisum.vault
mcli set account permission $ops_con active --add-code







echo "==== 1) 部署/更新 cisum.vault ===="
mreg flon $ops_con flonian
mtran flonian $ops_con "100 FLON"
mset $ops_con cisum.vault
mcli set account permission $ops_con active --add-code

echo "==== 3) 初始化合约（设置 admin/treasury） ===="
mpush $ops_con init '[
  "'"${ops_admin}"'"
]' -p $ops_con

echo "==== 4) 配置 CISUM Token：允许用户转入 cisum.vault ===="
mpush cisum.token addconsumewl '["'"${ops_con}"'"]' -p cisum.token


echo "==== 6) 兑换积分（points）=== "
mpush cisum.token transfer '[
  "gahbnbehaskk",
  "cisum.vault",
  "10.0000 CISUM",
  "points:1001:order-1"
]' -p gahbnbehaskk

echo "==== 7) 直播间付费（livepay）=== "
mpush cisum.token transfer '[
  "gahbnbehaskk",
  "cisum.vault",
  "2.0000 CISUM",
  "livepay:1:1001:ref-2"
]' -p gahbnbehaskk

echo "==== 8) CISUM 积分兑换（cisumptxchg）=== "
mpush cisum.token transfer '[
  "gahbnbehaskk",
  "cisum.vault",
  "1.0000 CISUM",
  "cisumptxchg:100"
]' -p gahbnbehaskk

echo "==== 9) memo 不匹配应报错 ===="
mpush cisum.token transfer '[
  "gahbnbehaskk",
  "'"${ops_con}"'",
  "1.0000 CISUM",
  "badmemo"
]' -p gahbnbehaskk 　

echo "==== 10) livepay 缺 room_id 应报错 ===="
mpush cisum.token transfer '[
  "gahbnbehaskk",
  "'"${ops_con}"'",
  "1.0000 CISUM",
  "livepay"
]' -p gahbnbehaskk

echo "==== 11) cisumptxchg 缺 points 应报错 ===="
mpush cisum.token transfer '[
  "gahbnbehaskk",
  "'"${ops_con}"'",
  "1.0000 CISUM",
  "cisumptxchg"
]' -p gahbnbehaskk

echo "==== 12) 暂停后应拒绝入金 ===="
mpush $ops_con setpause '[false]' -p $ops_admin

mpush cisum.token transfer '[
  "gahbnbehaskk",
  "'"${ops_con}"'",
  "1.0000 CISUM",
  "points:1002:order-2"
]' -p gahbnbehaskk

mpush $ops_con setpause '[false]' -p $ops_admin
