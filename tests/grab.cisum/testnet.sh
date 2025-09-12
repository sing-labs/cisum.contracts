#!/bin/bash

## workdir: path root of repository


grab_con=grab23.cisum
mreg flon $grab_con flonian
mtran flonian $grab_con "100 FLON"
mset $grab_con grab.cisum
mcli set account permission $grab_con active --add-code



## env: see client.env
grab_contract="grab23.cisum"
point_contract="nest21.token"
ticket_contract="cvticket.nft"
show_contract="show24.cisum"
admin="flonian"
user="user1"

## create contract account
tcli system newaccount flon ${grab_contract} "${admin}@active" --fund-account "10.0 FLON"

## should cd to contract repository dir
tcli set contract ${grab_contract} ./build/contracts/grab.cisum -p ${grab_contract}@active


## initialize contract
tpush ${grab_contract} init "[\"${admin}\"]" -p ${grab_contract}@active

## config point contract
tpush ${grab_contract} cfgpoint "[\"${point_contract}\"]" -p "${admin}@active"
## config point contract
tpush ${grab_contract} cfgticket "[\"${ticket_contract}\"]" -p "${admin}@active"

## addwhitelist in point contract
tpush ${point_contract} addwhitelist "[\"${contract}\"]" -p ${point_contract}@active

## addrushsale
tpush ${grab_contract} addrushsale "[
[1000000001], [2000000001], \"2025-08-20T00:00:00.000\",\"2026-08-20T00:00:00.000\",\"100.0000 NESTAR\", 1
, 10000]" -p "${admin}@active"

## get rush sale id
tcli get table ${grab_contract} ${grab_contract} rushsales -l 1 -r | jq '.rows[0].id'

## transfer ticket to grab contract
tpush ${ticket_contract} transfer "[\"${show_contract}\", \"${grab_contract}\", [[1000.0000, [2000000001]], \"add:1\"]]" -p ${show_contract}@active

## transfer point to contract and grab a ticket
tpush ${point_contract} transfer "[\"${user}\", \"${grab_contract}\", \"100.0000 NESTAR\", \"grab:1\"]" -p ${user}@active



mpush $grab_con init '["flonian"]' -p $grab_con
#增加消费白名单
mpush $nestar_token  addconsumewl '["grab23.cisum"]'   -p $nestar_token
mpush  cvticket.nft  addwhitelist '["grab23.cisum"]'   -p cvticket.nft


mpush  grab23.cisum  delrushsale '["21010001000010026",true]' -p flonian

mpush nest21.token  transfer '["gahbnbehaskk","grab23.cisum","200.0000 NESTAR","grab:16:a1799ae8e1ea62a20a5f56710342a401"]' -p gahbnbehaskk
mpush nest21.token  transfer '["ipowner.111","grab23.cisum","200.0000 NESTAR","grab:16:a1799ae8e1ea62a20a5f56810342a401"]' -p ipowner.111
mpush nest21.token  transfer '["myadmin","grab23.cisum","200.0000 NESTAR","grab:16:a1799ae8e1ea62a20a5f56910342a401"]' -p myadmin
mpush nest21.token  transfer '["nes11.issuer","grab23.cisum","200.0000 NESTAR","grab:16:a1799ae8e1ea62a20a5f57010342a401"]' -p nes11.issuer



for i in {1..1000}; do
  echo "第 $i 次执行..."
  mpush nest21.token  transfer '["gahbnbehaskk","grab23.cisum","200.0000 NESTAR","grab:16:a1799ae8e1ea62a20a5f56710342a401"]' -p gahbnbehaskk
  sleep 0.5  # 每次间隔 0.5 秒，避免节点压力过大
done


mpush  grab23.cisum  setrushsale '[10,1000,1000,null]' -p flonian


mpush grab23.cisum cfgpoint '["nest21.token"]' -p flonian
mpush grab23.cisum settoken '["4,NESTAR","nest21.token"]' -p grab23.cisum