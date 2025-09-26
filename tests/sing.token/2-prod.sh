#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc


sing_token=sing.token
mreg flon $sing_token flonian
mtran flonian $sing_token "100 FLON"
mset $sing_token sing.token
mcli set account permission $sing_token active --add-code



mpush $sing_token create '["flonian","10000000000.00000000 SING"]' -p $sing_token
mpush $sing_token issue '["flonian","100000000.00000000 SING","1st issue"]' -p flonian





mpush $sing_token create '["flonian","10000000000.00000000 MUSIC"]' -p $sing_token
mpush $sing_token issue '["flonian","100000000.00000000 MUSIC","1st issue"]' -p flonian

