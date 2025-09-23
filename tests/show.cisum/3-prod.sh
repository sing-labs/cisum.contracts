#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

show_con=show.cisum
mreg flon $show_con flonian
mtran flonian $show_con "100 FLON"
mset $show_con show.cisum
mcli set account permission $show_con active --add-code



mpush ticket.cvnft  addwhitelist '["'"${show_con}"'"]'   -p ticket.cvnft

mpush $show_con init '["flonian", "ticket.cvnft"]' -p $show_con


