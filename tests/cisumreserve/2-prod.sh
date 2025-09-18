


cisumreserve_con=cisumreserve
mreg flon $cisumreserve_con flonian
mtran flonian $cisumreserve_con "100 FLON"
mset $cisumreserve_con cisumreserve
mcli set account permission $cisumreserve_con active --add-code

admin=flonian
mpush $cisumreserve_con init '["'"$admin"'", 30]' -p $cisumreserve_con

