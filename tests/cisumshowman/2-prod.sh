#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

ops_con=cisumshowman
mreg flon $ops_con flonian
mtran flonian $ops_con "100 FLON"
mset $ops_con cisumshowman
mcli set account permission $ops_con active --add-code










mpush $ops_con publishshow '[
  "flonian",
  {
    "show_id": 1766630423513,
    "category": "concert",
    "ticket_transferable": true,
    "ticket_refundable": true,
    "show_started_at": "2025-12-22T19:30:00",
    "show_ended_at":   "2025-12-31T22:00:00",
    "show_name": "CISUM Live in Shanghai",
    "show_address": ""
  },
  [
    {
      "ticket_id": 2101025792788,
      "token_uri": "ipfs://free_ticket625_metadata",
      "ticket_type": "vip",
      "price": "1.0000 CISUM",
      "price_usdt": "0.0000 USDT",
      "pay_ticket": {"amount": 0,"symbol": { "nid": 0 }},
      "total_count": 100,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-12-25T03:39:52",
      "sale_ended_at": "2025-12-27T03:39:52",
      "win_ratio": 2000,
      "max_grabs_per_user": 1,
      "activity_type": "rushsale"
    },
    {
      "ticket_id": 2101025081754,
      "token_uri": "ipfs://free_ticket626_metadata",
      "ticket_type": "free",
      "price": "2.0000 CISUM",
      "price_usdt": "0.0000 USDT",
      "pay_ticket": {"amount": 0,"symbol": { "nid": 0 }},
      "total_count": 200,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-12-25T03:40:07",
      "sale_ended_at": "2025-12-27T03:40:07",
      "win_ratio": 5000,
      "max_grabs_per_user": 1,
      "activity_type": "rushsale"
    }
  ]
]' -p   flonian




mpush $ops_con addupgrades '[
  "flonian",
  1766630423513,
  [
    {
      "ticket_id": 2101025792788,
      "token_uri": "",
      "ticket_type": "vip",
      "price": "1.0000 CISUM",
      "price_usdt": "0.0000 USDT",
      "pay_ticket": {
        "amount": 0,
        "symbol": { "nid": 0 }
      },
      "total_count": 100,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-12-25T03:39:52",
      "sale_ended_at": "2025-12-27T03:39:52",
      "win_ratio": 2000,
      "max_grabs_per_user": 1,
      "activity_type": "rushsale"
    },
    {
      "ticket_id": 2101025081754,
      "token_uri": "",
      "ticket_type": "free",
      "price": "2.0000 CISUM",
      "price_usdt": "0.0000 USDT",
      "pay_ticket": {
        "amount": 0,
        "symbol": { "nid": 0 }
      },
      "total_count": 200,
      "prerequisite_ticket_id": 0,
      "sale_started_at": "2025-12-25T03:40:07",
      "sale_ended_at": "2025-12-27T03:40:07",
      "win_ratio": 5000,
      "max_grabs_per_user": 1,
      "activity_type": "rushsale"
    }
  ]
]' -p   flonian