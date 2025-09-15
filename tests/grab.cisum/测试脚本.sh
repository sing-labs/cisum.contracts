#!/bin/bash
set -e

grab_con=grab24.cisum

echo "==== 环境初始化 ===="
mreg flon $grab_con flonian
mtran flonian $grab_con "100 FLON"
mset $grab_con grab.cisum
mcli set account permission $grab_con active --add-code


echo "==== 1) 初始化合约 ===="
mpush $grab_con init '["flonian"]' -p $grab_con

echo "==== 2) 配置依赖合约 ===="
mpush $grab_con cfgpoint  '["nest21.token"]'  -p flonian
mpush $grab_con cfgticket '["cvticket.nft"]' -p flonian


echo "==== 3) 创建 rush sale ===="
mpush $grab_con addrushsale '[
  20250067,
  21010001000010148,
  "2025-11-01T00:00:00",
  "2025-12-01T00:00:00",
  "1.0000 NESTAR",
  1,
  5000
]' -p flonian

# 假设 rush_sale_id = 1 （实际要通过 get table 确认）
RUSH_ID=1


echo "==== 4) 往 rush sale 投放门票 ====" 无库存报错
mpush show24.cisum issuetograb '[
  "grab23.cisum",
  { "amount": 1, "symbol": { "value": "21010001000010134" } },
  "add:'$RUSH_ID':20250056"
]' -p flonian


#转账的币种不对报错
mpush show24.cisum issuetograb '[
  "grab23.cisum",
  { "amount": 1, "symbol": { "value": "21010001000010135" } },
  "add:'$RUSH_ID':20250056"
]' -p flonian






echo "==== 5) 给 user1  空投 NESTAR ===="
mpush nest21.token transfer '["nes11.issuer","user1","400.0000 NESTAR","airdrop"]' -p nes11.issuer



echo "==== 6) user1 抢票（应成功或失败随机） ===="
mpush nest21.token transfer '[
  "user1",
  "grab23.cisum",
  "1.0000 NESTAR",
  "grab:'$RUSH_ID':GID_001"
]' -p user1

echo "==== 7) user1 再次用相同 grab_id 抢票（应报错：duplicate grab_id） ===="
mpush nest21.token transfer '[
  "user1",
  "grab23.cisum",
  "1.0000 NESTAR",
  "grab:'$RUSH_ID':GID_001"
]' -p user1 || echo "✅ duplicate grab_id correctly rejected"


echo "==== 8) user2 抢票 ===="
mpush nest21.token transfer '[
  "user1",
  "grab23.cisum",
  "200.0000 NESTAR",
  "grab:'$RUSH_ID':GID_002"
]' -p user1


echo "==== 9) 设置 rush sale 参数（限购+中奖率+结束时间） ===="
mpush $grab_con setrushsale '[
  '$RUSH_ID',
  5,
  8000,
  "2025-12-15T00:00:00"
]' -p flonian


echo "==== 10) 单独修改中奖率 ===="
mpush $grab_con setrushsale '[
  '$RUSH_ID',
  null,
  3000,
  null
]' -p flonian

echo "==== 11) 单独修改结束时间（小于当前时间应报错） ===="
mpush $grab_con setrushsale '[
  '$RUSH_ID',
  null,
  null,
  "2020-01-01T00:00:00"
]' -p flonian || echo "✅ ended_at < now correctly rejected"


echo "==== 12) 删除 rush sale（非强制，活动未结束应报错） ===="
mpush $grab_con delrushsale '[
  '$RUSH_ID',
  false
]' -p flonian || echo "✅ cannot delete active rush sale"


echo "==== 13) 删除 rush sale（强制删除） ===="
mpush $grab_con delrushsale '[
  '$RUSH_ID',
  true
]' -p flonian


echo "==== 14) 删除用户订单（rush sale 已删除，才允许） ===="
mpush $grab_con delusers '[
  '$RUSH_ID',
  100
]' -p flonian