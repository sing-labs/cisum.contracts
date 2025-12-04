#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc


badge_ntoken=badge.cvnft
mreg flon $badge_ntoken flonian
mtran flonian $badge_ntoken "100 FLON"
mset $badge_ntoken ticket.cvnft
mcli set account permission $badge_ntoken active --add-code






mpush badge.cvnft  settokenuri '[4000000001,"https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreignrxe7nqwtfi7pkld2f5znaowvxoactzqvwpvnmgouz7nk2iaj7i"]'   -p badge.cvnft

mpush badge.cvnft  settokenuri '[4000000002,"https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreicqw3lb7cn5v3ivglfb4fv4w3w2riz2vtwvui3now3y2ej6vxbkfq"]'   -p badge.cvnft

mpush badge.cvnft  settokenuri '[4000000003,"https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreia3isbdkgpg2dbcwbxk2quarpvgowx6eaealzah64dp5csylofkqy"]'   -p badge.cvnft

mpush badge.cvnft  settokenuri '[4000000004,"https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreibmfjzwkh4ef4q7pawph4p5sff2i3fmnevabwst5onzfssdlsrzhe"]'   -p badge.cvnft


