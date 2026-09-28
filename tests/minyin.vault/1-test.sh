#!/bin/bash
set -e

ops_con=minyin.vault
ops_admin=flonian
user=gahbnbehaskk


echo "==== 1) 部署/更新 ${ops_con}（使用 minyin.vault） ===="
mreg flon "$ops_con" flonian
mtran flonian "$ops_con" "100 FLON"
mset "$ops_con" minyin.vault
mcli set account permission "$ops_con" active --add-code

echo "==== 2) 初始化合约（设置 admin，并解除暂停） ===="
mpush "$ops_con" init '["'"${ops_admin}"'"]' -p "$ops_con"

echo "==== 3) 配置 CISUM Token：允许用户转入 ${ops_con} ===="
mpush cisum.token addconsumewl '["'"${ops_con}"'"]' -p cisum.token

echo "==== 4) 民音投票（minyinvote，带附加内容） ===="
mpush cisum.token transfer '[
  "'"${user}"'",
  "'"${ops_con}"'",
  "10.0000 CISUM",
  "minyinvote:1001:order-1"
]' -p "$user"

echo "==== 5) 民音投票（minyinvote，不带附加内容） ===="
mpush cisum.token transfer '[
  "'"${user}"'",
  "'"${ops_con}"'",
  "2.0000 CISUM",
  "minyinvote"
]' -p "$user"

echo "==== 6) 民音投票（minyinvote，多个附加字段） ===="
mpush cisum.token transfer '[
  "'"${user}"'",
  "'"${ops_con}"'",
  "1.0000 CISUM",
  "minyinvote:100:'"${user}"':1"
]' -p "$user"

