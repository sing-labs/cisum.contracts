# PoE 批量余额空投

`batchairdrop(oper, drops, memo)` 使用 PoE 已有 CISUM 余额发放奖励，不增发。
复用 `global.operators` 白名单；操作者必须同时提供链上授权。
原 `global`、`rewardacts` 表结构不变，无需重新初始化。

以下命令在已配置好节点连接和钱包的本地环境中执行。
执行前先部署当前版本的 PoE WASM/ABI，并解锁具有所需签名权限的钱包。
将 `airdropadmin`、`issueracct`、`alice`、`bob` 替换为实际已创建的操作员、充值账户和收款账户。
充值账户需具有转出权限，且至少持有 `100.0000 CISUM`。

```bash
# 根据现有配置补充代码权限和转账白名单，再添加空投操作员。
fucli set account permission poe.cisum active --add-code -p poe.cisum@active
fucli push action cisum.token addwhitelist '["poe.cisum"]' -p cisum.token@active
fucli push action poe.cisum addoperator '["airdropadmin"]' -p poe.cisum@active

# 充值会增加 available_points。
fucli push action cisum.token transfer \
  '["issueracct","poe.cisum","100.0000 CISUM","空投充值"]' -p issueracct@active
fucli push action poe.cisum batchairdrop \
  '["airdropadmin", [{"beneficiary":"alice","quantity":"1.0000 CISUM"},{"beneficiary":"bob","quantity":"2.0000 CISUM"}], "活动奖励"]' \
  -p airdropadmin@active
```
