ops_con=ops21.cisum
mreg flon $ops_con flonian
mtran flonian $ops_con "100 FLON"
mset $ops_con cisumshowman
mcli set account permission $ops_con active --add-code


mpush $show_con addshowadm '["ops21.cisum"]' -p flonian


mpush $rolemanage_con grantrole '["show24.cisum","flonian","ops21.cisum","showadmin"]' -p flonian

ops21.cisum

# ========= 一把提交：cisumshowman::publishshow =========
mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250283,
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
      "ticket_id": 21010001000030300,
      "token_uri": "ipfs://free_ticket292_metadata",
      "ticket_type": "free",
      "price": "6000.0000 SONG",
      "price_usdt": "0 USDT",
      "total_count": 100,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-08-01T00:00:00",
      "sale_ended_at":   "2025-10-01T20:00:00",
      "win_ratio":1000,
      "max_grabs_per_user":10000
    },
    {
      "ticket_id": 21010001000030301,
      "token_uri": "ipfs://vip_ticket292_metadata",
      "ticket_type": "vip",
      "price": "199.00 USDT",
      "price_usdt": "199 USDT",
      "total_count": 2000,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-08-01T00:00:00",
      "sale_ended_at":   "2025-10-01T20:00:00",
      "win_ratio":0,
      "max_grabs_per_user":0
    }
  ]
]' -p flonian



mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 1758008184540,
        "category": "concert",
        "ticket_transferable": false,
        "ticket_refundable": false,
        "show_started_at": "2025-09-30T08:29:11",
        "show_ended_at": "2025-09-30T08:29:11",
        "show_name": "测试9月16日演出",
        "show_address": ""
  },
  [
    {
      "ticket_id": "2101025153580",
            "token_uri": "https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreieeznew2nyyezwll3rtd5n2m5xjcbo4r3t5usmz47iubowd2m2vuu",
            "ticket_type": "vip",
            "price": "10000 VND",
            "price_usdt": "10 USDT",
            "total_count": "100",
            "prerequisite_ticket_id": 0,
            "sale_started_at": "2025-09-16T08:29:16",
            "sale_ended_at": "2025-09-17T08:29:16",
            "win_ratio": 0,
            "max_grabs_per_user": 0
    },
    {
            "ticket_id": "2101025783564",
            "token_uri": "https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreib7dku4gtzormtkhboihs4jxpq526lzwbyxqx4ccegveq4wrzp64i",
            "ticket_type": "free",
            "price": "100.0000 NESTAR",
            "price_usdt": "0.0000 USDT",
            "total_count": "100",
            "prerequisite_ticket_id": 0,
            "sale_started_at": "2025-09-16T08:29:11",
            "sale_ended_at": "2025-09-17T08:29:11",
            "win_ratio": 1000,
            "max_grabs_per_user": 1
    }
  ]
]' -p flonian  -p $ops_con








mpush $show_con addchecker '[20250026,"flonian"]' -p flonian
mpush $show_con addchecker '[20250026,"myadmin"]' -p flonian

mpush $show_con addshowadm '["gahbnbehaskk"]' -p flonian



