#!/bin/bash

## workdir: path root of repository

## env: see client.env
grab_contract="grab15.cisum"
point_contract="nest15.token"
ticket_contract="cvticket.nft"
show_contract="show15.cisum"
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
tpush ${point_contract} transfer "[\"${user}\", \"${grab_contract}\", \"100.0000 NESTAR\", \"\"]" -p ${user}@active
