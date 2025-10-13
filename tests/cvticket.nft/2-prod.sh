#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

ticket_ntoken=ticket.cvnft
mreg flon $ticket_ntoken flonian
mtran flonian $ticket_ntoken "100 FLON"
mset $ticket_ntoken cvticket.nft
mcli set account permission $ticket_ntoken active --add-code


mpush $ticket_ntoken  addwhitelist '["ticket.cvnft"]'   -p $ticket_ntoken



