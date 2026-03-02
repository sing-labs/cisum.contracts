#!/bin/bash
set -e

ops_con=ops.cisum
ops_admin=flonian
user=gahbnbehaskk

echo "==== 1) 部署/更新 ops.cisum ===="
mreg flon $ops_con flonian
mtran flonian $ops_con "100 FLON"
mset $ops_con ops.cisum
mcli set account permission $ops_con active --add-code

echo "==== 3) 初始化合约（设置 admin/treasury） ===="
mpush $ops_con init '[
  "'"${ops_admin}"'"
]' -p $ops_con

echo "==== 4) 配置 CISUM Token：允许用户转入 ops.cisum ===="
mpush cisum.token addconsumewl '["'"${ops_con}"'"]' -p cisum.token


echo "==== 6) 兑换积分（points）=== "
mpush cisum.token transfer '[
  "gahbnbehaskk",
  "ops.cisum",
  "10.0000 CISUM",
  "points:1001:order-1"
]' -p gahbnbehaskk

echo "==== 7) 直播间付费（livepay）=== "
mpush cisum.token transfer '[
  "gahbnbehaskk",
  "ops.cisum",
  "2.0000 CISUM",
  "livepay:1:1001:ref-2"
]' -p gahbnbehaskk

echo "==== 8) memo 不匹配应报错 ===="
mpush cisum.token transfer '[
  "gahbnbehaskk",
  "'"${ops_con}"'",
  "1.0000 CISUM",
  "badmemo"
]' -p gahbnbehaskk 　

echo "==== 9) livepay 缺 room_id 应报错 ===="
mpush cisum.token transfer '[
  "gahbnbehaskk",
  "'"${ops_con}"'",
  "1.0000 CISUM",
  "livepay"
]' -p gahbnbehaskk

echo "==== 10) 暂停后应拒绝入金 ===="
mpush $ops_con setpause '[false]' -p $ops_admin

mpush cisum.token transfer '[
  "gahbnbehaskk",
  "'"${ops_con}"'",
  "1.0000 CISUM",
  "points:1002:order-2"
]' -p gahbnbehaskk

mpush $ops_con setpause '[false]' -p $ops_admin

