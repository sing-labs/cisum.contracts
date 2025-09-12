#!/bin/bash

## workdir: path root of repository


grab_con=grab23.cisum
mreg flon $grab_con flonian
mtran flonian $grab_con "100 FLON"
mset $grab_con grab.cisum
mcli set account permission $grab_con active --add-code


mpush $grab_con init '["flonian"]' -p $grab_con
#增加消费白名单
mpush $nestar_token  addconsumewl '["grab23.cisum"]'   -p $nestar_token
mpush  cvticket.nft  addwhitelist '["grab23.cisum"]'   -p cvticket.nft
mpush grab23.cisum cfgpoint '["nest21.token"]' -p flonian
mpush grab23.cisum cfgticket '["cvticket.nft"]' -p flonian
mpush grab23.cisum settoken '["4,NESTAR","nest21.token"]' -p grab23.cisum



mpush  grab23.cisum  delrushsale '["21010001000010026",true]' -p flonian

mpush nest21.token  transfer '["gahbnbehaskk","grab23.cisum","200.0000 NESTAR","grab:16:a1799ae8e1ea62a20a5f56710342a401"]' -p gahbnbehaskk
mpush nest21.token  transfer '["ipowner.111","grab23.cisum","200.0000 NESTAR","grab:16:a1799ae8e1ea62a20a5f56810342a401"]' -p ipowner.111
mpush nest21.token  transfer '["myadmin","grab23.cisum","200.0000 NESTAR","grab:16:a1799ae8e1ea62a20a5f56910342a401"]' -p myadmin
mpush nest21.token  transfer '["nes11.issuer","grab23.cisum","200.0000 NESTAR","grab:16:a1799ae8e1ea62a20a5f57010342a401"]' -p nes11.issuer



for i in {1..1000}; do
  echo "第 $i 次执行..."
  mpush nest21.token  transfer '["gahbnbehaskk","grab23.cisum","200.0000 NESTAR","grab:3:a1799ae8e1ea62a20a5f56710342a501"]' -p gahbnbehaskk
  sleep 0.5  # 每次间隔 0.5 秒，避免节点压力过大
done


mpush  grab23.cisum  setrushsale '[10,1000,1000,null]' -p flonian


