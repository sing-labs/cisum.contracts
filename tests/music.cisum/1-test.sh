music_token=music.cisum
mreg flon $music_token flonian
mtran flonian $music_token "100 FLON"
mset $music_token flon.token
mcli set account permission $music_token active --add-code



mpush $music_token create '["flonian","10000000000.00000000 MUSIC"]' -p $music_token
mpush $music_token issue '["flonian","100000000.00000000 MUSIC","1st issue"]' -p flonian
mtran -c $music_token flonian flon.xchain "1000000.00000000 MUSIC" "refuel"
mcli get currency stats $music_token MUSIC
mcli get currency balance $music_token flonian MUSIC




mpush $music_token transfer '["flonian", "gahbnbehaskk", "160000.00000000 MUSIC", "airdrop"]' -p flonian

mpush $music_token transfer '["gahbnbehaskk", "flonian", "60000.00000000 MUSIC", "airdrop"]' -p gahbnbehaskk