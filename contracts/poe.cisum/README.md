# PoE 批量余额空投

`batchairdrop(oper, drops, memo)` 使用 PoE 已有 CISUM 余额发放奖励，不增发。
复用 `global.operators` 白名单；操作者必须同时提供链上授权。
原 `global`、`rewardacts` 表结构不变，无需重新初始化。

```bash
# 部署更新后的 PoE WASM/ABI；根据现有配置补充代码权限和转账白名单。
cleos set account permission poe.cisum active --add-code -p poe.cisum@active
cleos push action cisum.token addwhitelist '["poe.cisum"]' -p cisum.token@active
cleos push action poe.cisum addoperator '["airdropadmin"]' -p poe.cisum@active

# issueracct 替换为实际发行账户或具有转出权限的账户；充值会增加 available_points。
cleos push action cisum.token transfer \
  '["issueracct","poe.cisum","100.0000 CISUM","空投充值"]' -p issueracct@active
cleos push action poe.cisum batchairdrop \
  '["airdropadmin", [{"beneficiary":"alice","quantity":"1.0000 CISUM"},{"beneficiary":"bob","quantity":"2.0000 CISUM"}], "活动奖励"]' \
  -p airdropadmin@active
```

每批 1～1000 条；单条为 4 位精度 CISUM，金额大于零且小于
`100000.0000 CISUM`；收款人必须存在且不能是本合约。统一备注最多 256 字节。
每条独立转账，不依赖 `rewardacts` 配置，奖励通知类型为 `airdrop`。
总额必须同时满足实际 token 余额和 `available_points` 余额要求。
发放总额从 `available_points` 扣减，累加到 `claimed_points`，`total_points` 不变。
任何校验、转账或通知失败整批回滚。没有业务去重，重复提交会再次发放。
原有操作员升级后也拥有该入口的发放权限。

测试链验收：验证缺少授权、非白名单、空列表、1001 条、金额错误、精度错误、
非法收款人和超长备注被拒绝。正常示例发放后供应量不变，PoE 余额及可用积分
各减少 3 CISUM，用户分别增加 1、2 CISUM，已发放积分增加 3 CISUM，总额度不变。
实际余额不足、账面可用积分不足或某笔转账失败时，整批余额与表数据不变。
移除操作员后无法发放，原有 batchclaim 仍按 rewardacts 配置正常执行。
