# PoH 注册奖励

注册、邀请奖励仍由 `registrar` 调用 `registreward` 发放。
批量空投迁移到 `poe.cisum::batchairdrop`，使用 PoE 原有操作员名单和充值奖励池。

本次移除 PoH 空投入口没有改变 `global` 和 `inviterfund` 的存储格式，无需重新 init。
若旧版本已部署，旧 `operators` 表不会自动删除，也不迁移到 PoE。
原 `global.cisum_issued` 累计值保留（含已有空投），后续余额空投计入 PoE 的
`claimed_points`，不再修改 PoH 的累计增发量。
