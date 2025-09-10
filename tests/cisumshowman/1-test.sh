ops_con=ops15.cisum
mreg flon $ops_con flonian
mtran flonian $ops_con "100 FLON"
mset $ops_con cisumshowman
mcli set account permission $ops_con active --add-code


mpush $show_con addshowadm '["ops15.cisum"]' -p flonian

# ========= 一把提交：cisumshowman::publishshow =========
mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 20250059,
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
      "ticket_id": 21010001000010140,
      "token_uri": "ipfs://free_ticket58_metadata",
      "ticket_type": "free",
      "price": "200.0000 NESTAR",
      "price_usd": "0.0000 USD",
      "total_count": 100,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-08-01T00:00:00",
      "sale_ended_at":   "2025-10-01T20:00:00",
      "win_ratio":1000,
      "max_grabs_per_user":10000
    },
    {
      "ticket_id": 21010001000010141,
      "token_uri": "ipfs://vip_ticket58_metadata",
      "ticket_type": "VIP",
      "price": "199.00 USDT",
      "price_usd": "199.00 USD",
      "total_count": 2000,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-08-01T00:00:00",
      "sale_ended_at":   "2025-10-01T20:00:00",
      "win_ratio":0,
      "max_grabs_per_user":0
    }
  ]
]' -p flonian  -p $ops_con



mpush $show_con addchecker '[20250026,"flonian"]' -p flonian
mpush $show_con addchecker '[20250026,"myadmin"]' -p flonian

mpush $show_con addshowadm '["gahbnbehaskk"]' -p flonian



