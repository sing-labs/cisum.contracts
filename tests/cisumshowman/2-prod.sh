#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

ops_con=cisumshowman
mreg flon $ops_con flonian
mtran flonian $ops_con "100 FLON"
mset $ops_con cisumshowman
mcli set account permission $ops_con active --add-code
