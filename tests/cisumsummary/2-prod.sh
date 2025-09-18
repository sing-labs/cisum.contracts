#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

cisumsummary_con=cisumsummary
mreg flon $cisumsummary_con flonian
mtran flon $cisumsummary_con "100.00000000 FLON"
mset $cisumsummary_con cisumsummary
mcli set account permission $cisumsummary_con active --add-code



# 添加 CISUM
mpush $cisumsummary_con addtoken '["cisum.token", "8,CISUM"]' -p $cisumsummary_con

# 添加 NESTAR
mpush $cisumsummary_con addtoken '["nestar.token", "4,NESTAR"]' -p $cisumsummary_con

# 添加 MUSIC
mpush $cisumsummary_con addtoken '["cisum.token", "8,MUSIC"]' -p $cisumsummary_con



#   mpush $cisumsummary_con view '["flonian"]' -p $cisumsummary_con


#   mpush $cisumsummary_con view '["flonian"]' --read -j





#tx=$(mcli push action -djs cisumsumma11 view '["flonian"]' --read --return-packed)
#curl -X POST --url https://t.flonscan.io/v1/chain/send_read_only_transaction -d "{\"transaction\": $tx}" | jq .


#curl -X POST https://t.flonscan.io/v1/chain/send_read_only_transaction \
#  -H "Content-Type: application/json" \
#  -d '{
#        "transaction": {
#          "signatures": [],
#          "compression": "none",
#          "packed_context_free_data": "",
#          "packed_trx": "e46f946869db1886dcb2000000000100000000045c01ce0000000000c095db0008000000601a37695c00"
#       }
#      }'