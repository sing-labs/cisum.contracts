#!/bin/bash
set -e
set -x

# ===== env.sh =====
export nestar_token=nest21.token   # NESTAR 主合约
export issuer_owner=nes11.issuer   # 发行者 owner
export user_owner=gahbnbehaskk     # 普通用户
export user_admin=myadmin          # 管理员
export artist_contract=art14.token # 艺人合约
export badgestore_contract=badgestore11
export GRAB_CON=grab23.cisum       # grab合约

# 预置 FLON 资金 & 注册账号
mreg flon $nestar_token flonian
mtran flonian $nestar_token "100 FLON"
mset $nestar_token nestar.token
mcli set account permission $nestar_token active --add-code

mreg flon $issuer_owner flonian
mtran flonian $issuer_owner "100 FLON"

mreg flon $user_owner flonian
mtran flonian $user_owner "100 FLON"

mreg flon $user_admin flonian
mtran flonian $user_admin "100 FLON"

# ===== 基础配置 =====
mpush $nestar_token setissuer   '["'"${issuer_owner}"'"]' -p $nestar_token
mpush $nestar_token setcontract '["'"${artist_contract}"'"]' -p $nestar_token
mpush $nestar_token setadmin    '["'"${user_admin}"'"]'   -p $nestar_token
mpush $nestar_token setbadgestore '["'"${badgestore_contract}"'"]' -p $nestar_token

# ===== 创建与发行 =====
mpush $nestar_token create '["'"${issuer_owner}"'", "10000000000000.0000 NESTAR"]' -p $nestar_token
mpush $nestar_token issue  '["'"${issuer_owner}"'", "1000000000.0000 NESTAR", "bootstrap"]' -p $issuer_owner

# ===== 规则设置 =====
mpush $nestar_token setbrule '[0, "15000.0000 NESTAR",   {"nid":"4299369637478511"}, true]'  -p $nestar_token
mpush $nestar_token setbrule '[0, "150000.0000 NESTAR",  {"nid":"4299369637478512"}, true]'  -p $nestar_token
mpush $nestar_token setbrule '[0, "1500000.0000 NESTAR", {"nid":"4299369637478513"}, true]'  -p $nestar_token
mpush $nestar_token setbrule '[0, "15000000.0000 NESTAR",{"nid":"4299369637478514"}, true]'  -p $nestar_token

mpush $nestar_token delbrule '["0"]' -p $nestar_token

# ===== 转账基础 =====
mpush $nestar_token transfer '["'"${issuer_owner}"'", "'"${user_owner}"'", "500.0000 NESTAR", "airdrop"]' -p $issuer_owner
mpush $nestar_token transfer '["'"${issuer_owner}"'", "ipowner.111", "500.0000 NESTAR", "airdrop"]' -p $issuer_owner
mpush $nestar_token transfer '["'"${user_owner}"'", "ipowner.111", "5000.0000 NESTAR", "airdrop"]' -p $user_owner
mpush $nestar_token transfer '["'"${user_owner}"'", "artist.acc1", "150000.0000 NESTAR", "airdrop"]' -p $user_owner
mpush $nestar_token transfer '["ipowner.111", "artist.acc1", "15000.0000 NESTAR", "tip"]' -p ipowner.111



# ================= 补充测试 =================

# === 权限与幂等 ===
mpush $nestar_token setissuer   '["bad.actor"]'  -p $user_admin       # 应失败
mpush $nestar_token setissuer   '["'"${issuer_owner}"'"]' -p $nestar_token # 幂等

mpush $nestar_token setcontract '["bad.contract"]' -p $user_admin     # 应失败
mpush $nestar_token setcontract '["'"${artist_contract}"'"]' -p $nestar_token # 幂等

mpush $nestar_token setadmin '["fake.admin"]' -p $issuer_owner        # 应失败
mpush $nestar_token setadmin '["'"${user_admin}"'"]' -p $nestar_token # 幂等

mpush $nestar_token setbadgestore '["bad.store"]' -p $issuer_owner    # 应失败
mpush $nestar_token setbadgestore '["'"${badgestore_contract}"'"]' -p $nestar_token # 幂等

# === create / issue 边界 ===
mpush $nestar_token create '["'"${issuer_owner}"'", "1.0000 NESTAR"]' -p $issuer_owner # 应失败
mpush $nestar_token issue  '["'"${issuer_owner}"'", "0.0000 NESTAR", "zero-issue"]' -p $issuer_owner # 应失败
mpush $nestar_token issue  '["'"${issuer_owner}"'", "1.000 NESTAR", "bad-precision"]' -p $issuer_owner # 应失败

# === transfer 边界 ===
mpush $nestar_token transfer '["'"${issuer_owner}"'", "'"${user_owner}"'", "0.0000 NESTAR", "zero"]' -p $issuer_owner
mpush $nestar_token transfer '["'"${issuer_owner}"'", "'"${user_owner}"'", "-1.0000 NESTAR", "neg"]' -p $issuer_owner
mpush $nestar_token transfer '["'"${issuer_owner}"'", "'"${user_owner}"'", "1.0000 FLON", "sym-mismatch"]' -p $issuer_owner
mpush $nestar_token transfer '["'"${issuer_owner}"'", "'"${user_admin}"'", "999999999999.0000 NESTAR", "no-balance"]' -p $issuer_owner

LONG_MEMO=$(python3 -c "print('x'*300)")
mpush $nestar_token transfer '["'"${issuer_owner}"'", "'"${user_owner}"'", "1.0000 NESTAR", "'"$LONG_MEMO"'"]' -p $issuer_owner

# === setbrule / delbrule 补充 ===
mpush $nestar_token setbrule '[0, "12345.0000 NESTAR", {"nid":"5555555555555555"}, true]' -p $nestar_token  # 应失败 --在badge中不存在
mpush $nestar_token setbrule '[0, "888.0000 NESTAR", {"nid":"5555555555555555"}, true]' -p $nestar_token # 应失败
mpush $nestar_token setbrule '[0, "77.0000 NESTAR", {"nid":"4444444444444444"}, false]' -p $nestar_token
mpush $nestar_token delbrule '["7"]' -p $nestar_token # 应失败
mpush $nestar_token setbrule '[0, "1.0000 NESTAR", {"nid":"123"}, true]' -p $issuer_owner # 应失败

# === 艺人与白名单 ===
mpush $nestar_token transfer '["'"${user_owner}"'", "art14.token", "100.0000 NESTAR", "tip:unlisted"]' -p $user_owner  #应失败 -- nestar是不能转账
mpush $nestar_token transfer '["'"${user_owner}"'", "'"${badgestore_contract}"'", "1.0000 NESTAR", "to-badgestore"]' -p $user_owner   #应失败 -- nestar是不能转账

# === GRAB 场景补充 ===
#时间结束
mpush $nestar_token transfer '["'"${user_owner}"'", "'"${GRAB_CON}"'", "300.0000 NESTAR", "grab:6:f9b5a9e54eed4eafbd0302ce79f39gs3"]' -p $user_owner
#时间还没有开始
mpush $nestar_token transfer '["'"${user_owner}"'", "'"${GRAB_CON}"'", "300.0000 NESTAR", "grab:5:f9b5a9e54eed4eafbd0302ce79f39gs3"]' -p $user_owner
#金额不对
mpush $nestar_token transfer '["'"${user_owner}"'", "'"${GRAB_CON}"'", "300.0000 NESTAR", "grab:3:f9b5a9e54eed4eafbd0302ce79f39gs3"]' -p $user_owner

#授权不对
mpush $nestar_token transfer '["'"${user_owner}"'", "'"${GRAB_CON}"'", "1.0000 NESTAR", "grab:3:f9b5a9e54eed4eafbd0302ce79f39gs3"]' -p $user_admin

#正常转账
mpush $nestar_token transfer '["'"${user_owner}"'", "'"${GRAB_CON}"'", "1.0000 NESTAR", "grab:3:f9b5a9e54eed4eafbd0302ce79f39gs3"]' -p $user_owner

# === 账户存在性 ===
mpush $nestar_token transfer '["'"${issuer_owner}"'", "zzzzzzzzzzzzz", "1.0000 NESTAR", "no-account"]' -p $issuer_owner  #应失败 --  zzzzzzzzzzzzz 账号不对
mpush $nestar_token transfer '["'"${issuer_owner}"'", "'"${issuer_owner}"'", "1.0000 NESTAR", "self"]' -p $issuer_owner   #应失败 -- 不能转给自己

# === 切换 admin 测试 ===
mpush $nestar_token setadmin '["alt.admin"]' -p $nestar_token   # 应失败-账号不存在
mpush $nestar_token issue '["'"${issuer_owner}"'", "1.0000 NESTAR", "old-admin-issue"]' -p $user_admin # 应失败





mpush $nestar_token transfer '["'"${issuer_owner}"'", "'"${user_owner}"'", "0.0000 NESTAR", "zero"]' -p $issuer_owner