#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc



cisumreserve_con=cisumreserve
mreg flon $cisumreserve_con flonian
mtran flonian $cisumreserve_con "100 FLON"
mset $cisumreserve_con cisumreserve
mcli set account permission $cisumreserve_con active --add-code

oracle=flonian
mpush $cisumreserve_con init '["'"$oracle"'", 30]' -p $cisumreserve_con

