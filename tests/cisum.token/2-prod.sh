#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc


cisum_token=cisum.token
mreg flon $cisum_token flonian
mtran flonian $cisum_token "100 FLON"
mset $cisum_token cisum.token
mcli set account permission $cisum_token active --add-code



mpush $cisum_token create '["flonian","10000000000.00000000 SING"]' -p $cisum_token
mpush $cisum_token issue '["flonian","100000000.00000000 SING","1st issue"]' -p flonian





mpush $cisum_token create '["flonian","10000000000.00000000 MUSIC"]' -p $cisum_token
mpush $cisum_token issue '["flonian","100000000.00000000 MUSIC","1st issue"]' -p flonian

