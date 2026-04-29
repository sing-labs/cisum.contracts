#!/bin/bash
set -e

grab_con=grab.cisum

echo "==== 环境初始化 ===="
mreg flon $grab_con flonian
mtran flonian $grab_con "100 FLON"
mset $grab_con grab.cisum
mcli set account permission $grab_con active --add-code


echo "==== 1) 初始化合约 ===="
mpush $grab_con init '["flonian"]' -p $grab_con

echo "==== 2) 配置依赖合约 ===="
mpush $grab_con cfgpoint  '["nestar.token"]'  -p flonian
mpush $grab_con cfgticket '["ticket.cvnft"]' -p flonian



echo "==== 3) 创建 rush sale ===="
mpush $grab_con addrushsale '[
  "flonian",
  1760855632531,
  2101025921797,
  "2026-04-01T00:00:00",
  "2026-04-29T00:00:00",
  "1.0000 CISUM",
  10000,
  2000
]' -p flonian

# 假设 rush_sale_id = 1 （实际要通过 get table 确认）
RUSH_ID=139


echo "==== 4) 往 rush sale 投放门票 ====" 无库存报错
mpush show.cisum issuetograb '[
  "flonian",
  "grab.cisum",
  { "amount": 300, "symbol": { "nid": "2101025921797" } },
  "addrushsale:'$RUSH_ID':1760855632531"
]' -p flonian


#转账的币种不对报错
mpush show24.cisum issuetograb '[
  "grab23.cisum",
  { "amount": 1, "symbol": { "nid": "21010001000010135" } },
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






USERS_CSV="gahbnbehaskk,acctaa1aaa11,acctaa1aaa12,acctaa1aaa13,acctaa1aaa14,acctaa1aaa15,acctaa1aaa21,acctaa1aaa22,acctaa1aaa23,acctaa1aaa24,acctaa1aaa25,acctaa1aaa31,acctaa1aaa32,acctaa1aaa33,acctaa1aaa34,acctaa1aaa35,acctaa1aaa41,acctaa1aaa42,acctaa1aaa43,acctaa1aaa44,acctaa1aaa45,acctaa1aaa51,acctaa1aaa52,acctaa1aaa53,acctaa1aaa54,acctaa1aaa55,acctaa2aaa11,acctaa2aaa12,acctaa2aaa13,acctaa2aaa14,acctaa2aaa15,acctaa2aaa21,acctaa2aaa22,acctaa2aaa23,acctaa2aaa24,acctaa2aaa25,acctaa2aaa31,acctaa2aaa32,acctaa2aaa33,acctaa2aaa34,acctaa2aaa35,acctaa2aaa41,acctaa2aaa42,acctaa2aaa43,acctaa2aaa44,acctaa2aaa45,acctaa2aaa51,acctaa2aaa52,acctaa2aaa53,acctaa2aaa54,acctaa2aaa55,acctaa3aaa11,acctaa3aaa12,acctaa3aaa13,acctaa3aaa14,acctaa3aaa15,acctaa3aaa21,acctaa3aaa22,acctaa3aaa23,acctaa3aaa24,acctaa3aaa25,acctaa3aaa31,acctaa3aaa32,acctaa3aaa33,acctaa3aaa34,acctaa3aaa35,acctaa3aaa41,acctaa3aaa42,acctaa3aaa43,acctaa3aaa44,acctaa3aaa45,acctaa3aaa51,acctaa3aaa52,acctaa3aaa53,acctaa3aaa54,acctaa3aaa55,acctaa4aaa11,acctaa4aaa12,acctaa4aaa13,acctaa4aaa14,acctaa4aaa15,acctaa4aaa21,acctaa4aaa22,acctaa4aaa23,acctaa4aaa24,acctaa4aaa25,acctaa4aaa31,acctaa4aaa32,acctaa4aaa33,acctaa4aaa34,acctaa4aaa35" \
RUSH_SALE_ID=139 \
AMOUNT="1.0000 CISUM" \
CONCURRENCY=10 \
python3 tests/grab.cisum/run_grab_orders.py