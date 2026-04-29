#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

stor_con=badgecvstore
badge_ntoken=badge.cvnft
rolemanage_con=cisum.auth
admin=flonian

submitter=${1:-flonian}
badge_nid=${2:-5000000004}
token_uri=${3:-"https://coral-reasonable-spider-53.mypinata.cloud/ipfs/bafkreibqffnhc6y4nvvsylduiezidzy6l6yum2rals7oq7g7plj7hp7gwm"}
max_supply=${4:-10000000000}
issue_amount=${5:-50000}



mpush $stor_con createbadge '[
  "'"${submitter}"'",
  '"${max_supply}"',
  {"nid":"'"${badge_nid}"'"},
  "'"${token_uri}"'",
  '"${issue_amount}"',
  "create badge through cvbadgestore"
]' -p $submitter
