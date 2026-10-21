# 多代币批量空投

`cisumairdrop` 使用预充值余额发放 EOSIO/FLON 风格的同质化代币，不铸币。
支持标准 `transfer(from,to,quantity,memo)`、`accounts` 余额表和 `stat` 供应表的代币合约。
例如 `cisum.token` 的 `4,CISUM`、`sing.token` 的 `8,SING`。不适用于 NFT 或其他接口不兼容的代币。

每批 1–200 个不同收款账户，每人可以设置不同金额。同一批次使用同一个代币合约、符号和精度，
不同批次可以发不同代币。所有转账成功才提交，任何一笔失败都会回滚整个批次。

## 部署和充值

以下使用项目现有的 `mcli` / `mpush` 辅助命令。先创建 `cisumairdrop` 账户并准备 RAM、CPU、NET。
使用 FLON CDT 构建项目后，部署 `build/contracts/cisumairdrop` 下的 WASM 和 ABI。
将 `adminacct`、`issueracct`、`alice`、`bob` 替换为实际存在的账户。

```bash
mcli set contract cisumairdrop build/contracts/cisumairdrop -p cisumairdrop@active
mcli set account permission cisumairdrop active --add-code -p cisumairdrop@active
mpush cisumairdrop init '["adminacct"]' -p cisumairdrop@active

# CISUM 要求转出白名单，需要 cisum.token 或其管理员授权。
mpush cisum.token addwhitelist '["cisumairdrop"]' -p cisum.token@active
mpush cisum.token transfer '["issueracct","cisumairdrop","1000.0000 CISUM","airdrop funding"]' -p issueracct@active

# 另一种代币单独充值，其权限规则由对应代币合约决定。
mpush sing.token transfer '["issueracct","cisumairdrop","100.00000000 SING","airdrop funding"]' -p issueracct@active
```

`--add-code` 允许合约代码以自身权限执行内部转账。合约不维护独立充值账本，直接读取所选代币合约的余额。
CISUM 充值方需具备转出资格；不要仅为充值而把空投合约加入消费白名单，否则充值会按消费累计。
本合约不额外维护代币白名单，代币合约由管理员或合约账户在发放时指定。

## 批量发放

接口：`batchdrop(batch_id, token_contract, token_symbol, recipients, memo)`。
`token_symbol` 使用 `精度,符号` 格式，每批仅指定一次。
每条收款项只传 `to` 和 `quantity`；`quantity` 是不带币种的十进制数量字符串，例如 `"10"`、`"0.5"`。
合约还会读取所选代币的 `stat` 表，验证实际精度。
数量按完整代币单位解释，不是最小单位：`"10"` 配合 `4,CISUM` 表示 `10.0000 CISUM`。
小数位不足时补零，超过精度时拒绝，不进行四舍五入；不接受正负号、科学计数法、空格或币种后缀。
请使用 JSON 字符串传数量，避免浮点数误差；数量字符串最长 40 字节。

```bash
# CISUM 空投
mpush cisumairdrop batchdrop '[1,"cisum.token","4,CISUM",[{"to":"alice","quantity":"10"},{"to":"bob","quantity":"20"}],"campaign-cisum"]' -p adminacct@active

# SING 空投：同一个空投合约，使用另一个批次号
mpush cisumairdrop batchdrop '[2,"sing.token","8,SING",[{"to":"alice","quantity":"1"},{"to":"bob","quantity":"2"}],"campaign-sing"]' -p adminacct@active
```

- 管理员或合约账户授权发放，接收者无需签名。
- 资产按代币合约 + 符号 + 精度区分；不同代币合约发行的同名代币相互独立。
- `batch_id` 为正 uint64，所有代币共享编号空间，成功后永久保留且不能重用，失败后可原编号重试。
- 同一账户可参与不同批次；不实现跨批次“每人限领一次”。
- 不存在账户、重复收款账户、给自身发放、非正金额、数量格式不合法、超出币种精度、总额溢出、余额不足均会拒绝。
- 所选代币不存在或空投账户尚无该币种余额行时，也会失败。
- `memo` 最多 256 字节，原样写入每笔转账和批次记录。
- 批次记录及转账新建的余额行由空投合约支付 RAM；200 条是条数上限，实际受链资源和代币合约规则限制。

```bash
mpush cisumairdrop setpause '[true]' -p adminacct@active
mpush cisumairdrop setpause '[false]' -p adminacct@active
mpush cisumairdrop setadmin '["newadmin"]' -p adminacct@active
```

`init` 只能执行一次。管理员或合约账户可更换管理员、暂停或恢复。暂停只阻止批量发奖，不阻止充值。

## 查询批次

```bash
mpush cisumairdrop lastbatch '[]' -p adminacct@active
mpush cisumairdrop getbatch '[1]' -p adminacct@active
mcli get table cisumairdrop cisumairdrop batches
```

查询接口是只读的，不要求管理员权限。示例签名用于普通交易提交，返回值在执行结果中查看，
需要节点和客户端支持 action 返回值，也可直接查表。

`lastbatch` 返回所有代币批次中的最大编号，无记录返回 0；允许乱序编号，所以不代表时间上最后一批。
`getbatch` 返回 `batch_id`、`executor`、`total`、`recipient_count`、`memo`、`created_at`、`token_contract`。
`total` 包含符号和精度。编号为 0 或不存在时查询报错。

## 升级已有部署

重新编译并同时部署 WASM 和 ABI，无需重新初始化管理员。
`recipients[].quantity` 从带币种的 `asset` 改为数量字符串，调用端必须更新。
例如旧的 `"10.0000 CISUM"` 改为 `"10"`，批次参数仍传 `"4,CISUM"`。

批次表在原有字段末尾追加 `binary_extension<name> token_contract`，保留旧记录的二进制布局。
旧 CISUM 记录没有该字段时，`getbatch` 自动返回 `cisum.token`；直接查表时该字段可能缺失。
旧编号继续占用，不能为其他代币重复使用。升级无需删除历史记录。

## 测试网集成验证

`tests/cisumairdrop/integration.py` 使用标准库调用 FLON/EOSIO 风格 CLI。
仅在独立测试环境运行：会实际发放测试币，并修改管理员及暂停状态。
需要已部署但未初始化的新合约、签名权限、两个代币的转出资格，至少 1 CISUM 和 10 个第二币种单位。
`adminacct`、`alice`、`bob` 和合约账户须互不相同。

```bash
python3 tests/cisumairdrop/integration.py --cli fucli \
  --url http://127.0.0.1:8888 --contract cisumairdrop \
  --admin adminacct --alice alice --bob bob \
  --other-token-contract sing.token --other-symbol 8,SING
```

覆盖两种代币发放及余额隔离、跨代币批次去重、数量解析/错误精度拒绝、错误代币合约、权限和批量限制、
批次查询、乱序编号、失败状态不变、暂停和管理员变更。
第二币种也可指定为其他合约发行的 `4,CISUM`，以验证同名代币合约隔离。
