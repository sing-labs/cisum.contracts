pop_con=pop11.cisum
mreg flon $pop_con flonian
mtran flonian $pop_con "100 FLON"
mset $pop_con pop.cisum
mcli set account permission $pop_con active --add-code


mpush $pop_con mine '["pop11.cisum","Registration Reward:1001"]' -p $pop_con


