# PoH CISUM 批量空投

`addoperator` / `deloperator` 仅允许合约账户授权，维护独立的 `operators`
单例表。初始名单为空；`registrar` 和合约账户不会自动获得空投资格。
现有 `registreward` 仍只允许 `registrar` 调用。

```bash
cleos push action poh.cisum addoperator '["airdropadmin"]' -p poh.cisum@active
cleos get table poh.cisum poh.cisum operators
cleos push action poh.cisum batchairdrop \
  '["airdropadmin", [{"beneficiary":"alice","quantity":"1.0000 CISUM"},{"beneficiary":"bob","quantity":"2.0000 CISUM"}], "活动奖励"]' \
  -p airdropadmin@active
cleos push action poh.cisum deloperator '["airdropadmin"]' -p poh.cisum@active
```

账户名需要替换为实际存在的账户。当前 `cisum.token` 将 PoH 账户固定为
`poh.cisum`，部署到其他账户不会自动获得增发权限。合约的 `active` 权限需要
允许自身的 `eosio.code` 执行内联 action，转账也须满足 token 的权限规则。

每批接受 1～1000 条，每条金额必须为正数且小于 10 CISUM，使用 4 位精度。
整批共用一个 memo（增发、转账和通知均使用它），不超过 256
字节，收款人必须存在且不能是本合约。重复收款人不会合并。
合约先向自身增发整批总额，再逐条转账并发送 `notifyreward`，奖励类型为
`airdrop`。新增金额计入 `global.cisum_issued`，不受 SING 的 `max_issued`
限制，但受 token 的 `max_supply` 限制。任何失败都会回滚整批交易。

此接口没有业务去重或累计空投额度上限；操作员可在单条限制及 token 剩余额度内
指定金额，重复提交会再次增发。操作员密钥应按增发权限管理。

## 测试链验收

在测试链部署更新后的 WASM/ABI 后验证以下场景；不要直接在生产链试发。

- 非合约账户调用 `addoperator`、`deloperator` 应失败。
- 未加入名单的账户调用 `batchairdrop` 应失败；伪造 `oper` 而无其授权也应失败。
- 加入名单后，按上述示例空投：供应量增加 3 CISUM，两个用户分别增加
  1、2 CISUM，PoH 合约余额保持不变，`cisum_issued` 增加 3 CISUM。
- 空列表、1001 条、单条金额达到 10 CISUM、零/负金额、错误精度或币种、超长备注、不存在的收款人、
  合约自身收款均应失败；供应量、余额和累计值保持不变。
- 总额超过剩余供应量，或某条转账被 token 拒绝时，应整批回滚。
- 移除操作员后再次调用应失败；重复添加/移除操作员应正常执行。
- 升级前已有的 `global` 数据应保留，原 `registrar` 注册奖励权限不受影响。
