#!/bin/bash

## workdir: path root of repository


grab_con=grab21.cisum
mreg flon $grab_con flonian
mtran flonian $grab_con "100 FLON"
mset $grab_con grab.cisum
mcli set account permission $grab_con active --add-code



## env: see client.env
grab_contract="grab21.cisum"
point_contract="nest21.token"
ticket_contract="cvticket.nft"
show_contract="show23.cisum"
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
mpush $nestar_token  addconsumewl '["grab21.cisum"]'   -p $nestar_token

mpush  grab21.cisum  delrushsale '["21010001000010026",true]' -p flonian

mpush nest21.token  transfer '["gahbnbehaskk","grab21.cisum","200.0000 NESTAR","grab:10"]' -p gahbnbehaskk
mpush nest21.token  transfer '["ipowner.111","grab21.cisum","200.0000 NESTAR","grab:2"]' -p ipowner.111
mpush nest21.token  transfer '["myadmin","grab21.cisum","200.0000 NESTAR","grab:2"]' -p myadmin
mpush nest21.token  transfer '["nes11.issuer","grab21.cisum","200.0000 NESTAR","grab:2"]' -p nes11.issuer
