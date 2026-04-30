#!/bin/bash
set -e
shopt -s expand_aliases
source ~/.bashrc

stor_con=${STOR_CON:-badgecvstore}
badge_ntoken=${BADGE_NTOKEN:-badge.cvnft}
admin=${ADMIN:-flonian}
from_user=${FROM_USER:-flonian}
to_user=${TO_USER:-gahbnbehaskk}
badge_nid= 2101025684899




mpush $badge_ntoken addwhitelist '["'"${stor_con}"'"]' -p $badge_ntoken

mpush $badge_ntoken transfer '[
  "'"${stor_con}"'",
  "'"${from_user}"'",
  [{"amount":1,"symbol":{"nid":"'"${badge_nid}"'"}}],
  "store account whitelist transfer"
]' -p $stor_con


mpush $badge_ntoken transfer '[
  "'"${from_user}"'",
  "'"${to_user}"'",
  [{"amount":1,"symbol":{"nid":"'"${badge_nid}"'"}}],
  "should fail before nft whitelist"
]' -p $from_user




mpush $badge_ntoken setnftwhite '[
  '"${badge_nid}"',
  true
]' -p $badge_ntoken

mpush $badge_ntoken transfer '[
  "'"${from_user}"'",
  "'"${to_user}"'",
  [{"amount":1,"symbol":{"nid":"'"${badge_nid}"'"}}],
  "allowed by nft whitelist"
]' -p $from_user


