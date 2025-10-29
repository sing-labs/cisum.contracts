
########## 00_env_prep.sh ##########
# 依你现有做法（如已做可跳过）
ops_con=ops21.cisum
mreg flon $ops_con flonian
mtran flonian $ops_con "100 FLON"
mset $ops_con cisumshowman
mcli set account permission $ops_con active --add-code

# 让发布器成为 show 管理员（确保能调 newshow/newticket）
mpush $show_con addshowadm '["ops21.cisum"]' -p flonian


⸻

1) Happy path：一个免费票 + 一个 VIP 票

########## 10_publish_happy.sh ##########
mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250060,
    "category": "concert",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-10-02T19:30:00",
    "show_ended_at":   "2025-10-02T22:00:00",
    "show_name": "CISUM Live in Shanghai",
    "show_address": ""
  },
  [
    {
      "ticket_id": 21010001000010142,
      "token_uri": "ipfs://free_ticket59_metadata",
      "ticket_type": "free",
      "price": "200.0000 NESTAR",
      "price_usd": "0.0000 USDT",
      "total_count": 100,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-08-01T00:00:00",
      "sale_ended_at":   "2025-10-01T20:00:00",
      "win_ratio": 1000,
      "max_grabs_per_user": 10000
    },
    {
      "ticket_id": 21010001000010143,
      "token_uri": "ipfs://vip_ticket59_metadata",
      "ticket_type": "VIP",
      "price": "199.00 USDT",
      "price_usd": "199.00 USD",
      "total_count": 2000,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-08-01T00:00:00",
      "sale_ended_at":   "2025-10-01T20:00:00",
      "win_ratio": 0,
      "max_grabs_per_user": 0
    }
  ]
]' -p flonian

# 补充：为该演出添加核销员
mpush $show_con addchecker '[20250060,"flonian"]' -p flonian
mpush $show_con addchecker '[20250060,"myadmin"]' -p flonian


⸻

2) 重复发布同一个 show_id（应失败：show 已存在）

########## 20_publish_duplicate_show.sh ##########
mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250060,
    "category": "concert",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-10-02T19:30:00",
    "show_ended_at":   "2025-10-02T22:00:00",
    "show_name": "DUP TRY",
    "show_address": ""
  },
  []
]' -p flonian
# 预期：失败（show_id already exists）


⸻

3) 时间窗口非法：结束早于开始（应失败）

########## 30_bad_time_window.sh ##########
mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250061,
    "category": "concert",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-10-02T22:00:00",
    "show_ended_at":   "2025-10-02T19:30:00",
    "show_name": "Bad Times",
    "show_address": ""
  },
  [
    {
      "ticket_id": 21010001000010144,
      "token_uri": "ipfs://bad_time_ticket",
      "ticket_type": "VIP",
      "price": "99.00 USDT",
      "price_usd": "99.00 USD",
      "total_count": 100,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-08-01T00:00:00",
      "sale_ended_at":   "2025-10-01T20:00:00",
      "win_ratio": 0,
      "max_grabs_per_user": 0
    }
  ]
]' -p flonian
# 预期：失败（show_ended_at must be >= show_started_at）


⸻

4) 非授权 creator 调用（应失败：require_auth(creator)）

########## 40_unauthorized_creator.sh ##########
# 用 USER1 作为签名者，但 creator 写 flonian -> 应失败（没有 flonian 的授权）
mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250061,
    "category": "concert",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-10-05T19:00:00",
    "show_ended_at":   "2025-10-05T21:00:00",
    "show_name": "UNAUTH",
    "show_address": ""
  },
  []
]' -p $USER1
# 预期：失败（missing authority of flonian）


⸻

5) creator 不是 flonian，而是自己（有自己授权）：应通过

publishshow 只要求 require_auth(creator)，而调用 newshow/newticket 用的是合约自身权限；只要 ops_con 已是 show_admin，就能成功。

########## 50_creator_is_self_ok.sh ##########
mpush $ops_con publishshow '[
  "ops21.cisum",
  {
    "show_id": 20250062,
    "category": "festival",
    "ticket_transferable": true,
    "ticket_refundable": false,
    "show_started_at": "2025-10-10T10:00:00",
    "show_ended_at":   "2025-10-10T22:00:00",
    "show_name": "Creator Self OK",
    "show_address": "Somewhere"
  },
  [
    {
      "ticket_id": 21010001000010144,
      "token_uri": "ipfs://self_ok_ticket",
      "ticket_type": "free",
      "price": "0.0000 NESTAR",
      "price_usd": "0.00 USD",
      "total_count": 10,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-10-01T00:00:00",
      "win_ratio": 1000,
      "max_grabs_per_user": 3
    }
  ]
]' -p $ops_con
# 预期：成功


⸻

6) ticket 边界：total_count = 0（应失败）

########## 60_zero_total_count.sh ##########
mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250063,
    "category": "test",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-10-20T19:00:00",
    "show_ended_at":   "2025-10-20T21:00:00",
    "show_name": "ZeroCount",
    "show_address": ""
  },
  [
    {
      "ticket_id": 21010001000010144,
      "token_uri": "ipfs://zero_total",
      "ticket_type": "VIP",
      "price": "10.00 USDT",
      "price_usd": "10.00 USD",
      "total_count": 0,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-10-01T00:00:00",
      "win_ratio": 0,
      "max_grabs_per_user": 0
    }
  ]
]' -p flonian
# 预期：失败（createnft 需要正的 max_supply / 你的逻辑通常会拒绝 0）


⸻

7) 超长 token_uri（>256，应失败）

########## 70_long_token_uri.sh ##########
LONG_URI=$(python3 - <<'PY'
print("ipfs://" + "a"*260)
PY
)

mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250064,
    "category": "test",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-10-21T19:00:00",
    "show_ended_at":   "2025-10-21T21:00:00",
    "show_name": "LongURI",
    "show_address": ""
  },
  [
    {
      "ticket_id": 21010001000010145,
      "token_uri": "'"$LONG_URI"'",
      "ticket_type": "VIP",
      "price": "9.99 USDT",
      "price_usd": "9.99 USD",
      "total_count": 100,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-10-01T00:00:00",
      "win_ratio": 0,
      "max_grabs_per_user": 0
    }
  ]
]' -p flonian
# 预期：失败（token_uri too long）


⸻

8) 两个 ticket 使用同一 ticket_id（应失败：同 show 下 ticket 已存在）

########## 80_duplicate_ticket_id.sh ##########
mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250065,
    "category": "test",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-10-25T19:00:00",
    "show_ended_at":   "2025-10-25T21:00:00",
    "show_name": "DupTicket",
    "show_address": ""
  },
  [
    {
      "ticket_id": 21010001000010146,
      "token_uri": "ipfs://dup_a",
      "ticket_type": "VIP",
      "price": "19.99 USDT",
      "price_usd": "19.99 USD",
      "total_count": 50,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-10-01T00:00:00",
      "win_ratio": 0,
      "max_grabs_per_user": 0
    },
    {
      "ticket_id": 21010001000010146,
      "token_uri": "ipfs://dup_b",
      "ticket_type": "VIP",
      "price": "29.99 USDT",
      "price_usd": "29.99 USD",
      "total_count": 30,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-10-01T00:00:00",
      "win_ratio": 0,
      "max_grabs_per_user": 0
    }
  ]
]' -p flonian
# 预期：失败（ticket already exists in this show）


⸻

9) 免费票链路检查（抓阈值/人均限制等极值）

########## 90_free_ticket_extremes.sh ##########
mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250066,
    "category": "concert",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-10-30T19:00:00",
    "show_ended_at": "2025-10-30T21:00:00",
    "show_name": "FreeExtremes",
    "show_address": ""
  },
  [
    {
      "ticket_id": 21010001000010147,
      "token_uri": "ipfs://free_extreme",
      "ticket_type": "free",
      "price": "0.0000 NESTAR",
      "price_usd": "0.00 USD",
      "total_count": 1000,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at": "2025-10-29T00:00:00",
      "win_ratio": 10000,
      "max_grabs_per_user": 1
    }
  ]
]' -p flonian
#


⸻

10) 价格精度不匹配（若底层严格校验 symbol 精度，应失败）

########## 95_price_precision_mismatch.sh ##########
mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250067,
    "category": "concert",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-11-02T19:00:00",
    "show_ended_at":   "2025-11-02T22:00:00",
    "show_name": "PricePrecision",
    "show_address": ""
  },
  [
    {
      "ticket_id": 21010001000010148,
      "token_uri": "ipfs://pp_ticket",
      "ticket_type": "VIP",
      "price": "199.0 USDT",
      "price_usd": "199.00 USD",
      "total_count": 10,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-11-01T00:00:00",
      "win_ratio": 0,
      "max_grabs_per_user": 0
    }
  ]
]' -p flonian
# 预期：若合约严格校验，将失败；否则视底层 symbol 定义

mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250068,
    "category": "concert",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-11-02T19:00:00",
    "show_ended_at":   "2025-11-02T22:00:00",
    "show_name": "PricePrecision",
    "show_address": ""
  },
  [
    {
      "ticket_id": 21010001000010159,
      "token_uri": "ipfs://pp_ticket",
      "ticket_type": "free",
      "price": "199.0 USDT",
      "price_usd": "199.00 USD",
      "total_count": 10,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-11-01T00:00:00",
      "win_ratio": 0,
      "max_grabs_per_user": 0
    }
  ]
]' -p flonian





1️⃣ 正常流程（收费票 + 免费票）

ops_con=cisumshowman

mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250100,
    "category": "concert",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-10-02T19:30:00",
    "show_ended_at":   "2025-10-02T22:00:00",
    "show_name": "Test Concert Normal",
    "show_address": "Shanghai"
  },
  [
    {
      "ticket_id": 21010001000011001,
      "token_uri": "ipfs://paid_ticket1_metadata",
      "ticket_type": "vip",
      "price": "100.00000000 CISUM",
      "price_usdt": "100.000000 USDT",
      "total_count": 100,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-09-30T20:00:00",
      "win_ratio": 0,
      "max_grabs_per_user": 0
    },
    {
      "ticket_id": 21010001000011002,
      "token_uri": "ipfs://free_ticket1_metadata",
      "ticket_type": "free",
      "price": "10.0000 NESTAR",
      "price_usdt": "0 USDT",
      "total_count": 10,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-09-30T20:00:00",
      "win_ratio": 10000,
      "max_grabs_per_user": 1
    }
  ]
]' -p flonian


⸻

2️⃣ 非授权用户调用

mpush $ops_con publishshow '[
  "notadmin",
  {
    "show_id": 20250101,
    "category": "concert",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-10-03T19:30:00",
    "show_ended_at":   "2025-10-03T22:00:00",
    "show_name": "Test Unauthorized",
    "show_address": "Beijing"
  },
  []
]' -p notadmin
# 期望：报错 missing authority of notadmin


⸻

3️⃣ USDT 符号错误

mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250102,
    "category": "concert",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-10-04T19:30:00",
    "show_ended_at":   "2025-10-04T22:00:00",
    "show_name": "Test Wrong USDT",
    "show_address": "Guangzhou"
  },
  [
    {
      "ticket_id": 21010001000011003,
      "token_uri": "ipfs://wrong_usdt_ticket",
      "ticket_type": "vip",
      "price": "199.0000 USDT",
      "price_usdt": "199.0000 ABC",
      "total_count": 100,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-09-30T20:00:00",
      "win_ratio": 0,
      "max_grabs_per_user": 0
    }
  ]
]' -p flonian
# 期望：报错 price_usdt code must be USDT


⸻

4️⃣ CISUM 精度错误

mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250103,
    "category": "concert",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-10-05T19:30:00",
    "show_ended_at":   "2025-10-05T22:00:00",
    "show_name": "Test Wrong CISUM Precision",
    "show_address": "Shenzhen"
  },
  [
    {
      "ticket_id": 21010001000011004,
      "token_uri": "ipfs://cisum_precision_ticket",
      "ticket_type": "vip",
      "price": "100.0000 CISUM",
      "price_usdt": "100.00000000 USDT",
      "total_count": 50,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-09-30T20:00:00",
      "win_ratio": 0,
      "max_grabs_per_user": 0
    }
  ]
]' -p flonian
# 期望：报错 CISUM price precision must be 8


⸻

5️⃣ 免费票价格非零

mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250104,
    "category": "concert",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-10-06T19:30:00",
    "show_ended_at":   "2025-10-06T22:00:00",
    "show_name": "Test Free Ticket Wrong Price",
    "show_address": "Chengdu"
  },
  [
    {
      "ticket_id": 21010001000011005,
      "token_uri": "ipfs://free_ticket_wrong_price",
      "ticket_type": "free",
      "price": "1.00000000 CISUM",
      "price_usdt": "0 USDT",
      "total_count": 10,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-09-30T20:00:00",
      "win_ratio": 10000,
      "max_grabs_per_user": 1
    }
  ]
]' -p flonian
# 期望：报错 price symbol not allowed


⸻

6️⃣ 多票种（NESTAR + CISUM + Free）

mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250105,
    "category": "concert",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-10-07T19:30:00",
    "show_ended_at":   "2025-10-07T22:00:00",
    "show_name": "Test Multi Tickets",
    "show_address": "Hangzhou"
  },
  [
    {
      "ticket_id": 21010001000011006,
      "token_uri": "ipfs://nestar_ticket",
      "ticket_type": "nestar",
      "price": "100.0000 NESTAR",
      "price_usdt": "100.0000 USDT",
      "total_count": 100,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-09-30T20:00:00",
      "win_ratio": 0,
      "max_grabs_per_user": 0
    },
    {
      "ticket_id": 21010001000011007,
      "token_uri": "ipfs://cisum_ticket",
      "ticket_type": "cisum",
      "price": "50.00000000 CISUM",
      "price_usdt": "50.0000 USDT",
      "total_count": 200,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-09-30T20:00:00",
      "win_ratio": 0,
      "max_grabs_per_user": 0
    },
    {
      "ticket_id": 21010001000011008,
      "token_uri": "ipfs://free_ticket_multi",
      "ticket_type": "free",
      "price": "0.00000000 CISUM",
      "price_usdt": "0 USDT",
      "total_count": 20,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-09-30T20:00:00",
      "win_ratio": 5000,
      "max_grabs_per_user": 2
    }
  ]
]' -p flonian


⸻

7️⃣ NFT 铸造量检查

mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250106,
    "category": "concert",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-10-08T19:30:00",
    "show_ended_at":   "2025-10-08T22:00:00",
    "show_name": "Test NFT Supply",
    "show_address": "Nanjing"
  },
  [
    {
      "ticket_id": 21010001000011009,
      "token_uri": "ipfs://nft_supply_ticket",
      "ticket_type": "vip",
      "price": "99.0000 USDT",
      "price_usdt": "99.0000 USDT",
      "total_count": 5,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-09-01T00:00:00",
      "sale_ended_at":   "2025-09-30T20:00:00",
      "win_ratio": 0,
      "max_grabs_per_user": 0
    }
  ]
]' -p flonian
# 期望：CREATE_NFT 实际创建 50 张 NFT（5 * 10）
