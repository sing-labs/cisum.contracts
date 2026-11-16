# mall.cisum 合约接口说明

本文对应当前源码及 ABI，面向后台开发、钱包接入和运营人员。包含全部 15 个公开 action，以及转账通知入口。构建与部署步骤见 [README](README.md)。

## 底层合约模式补充

当前 `mall.nft` 已恢复逐 NFT 白名单和 setpolicy。商城 `createnft` 只创建模板、登记商品并记录商城销售配额，**不检查、不设置 NFT 转出权限**，也不调用 setpolicy 或 setxferwl。
商城不预检查策略表或发送白名单；转出权限由底层 transfer 判断，失败整笔交易回滚。权限应按底层合约规则独立配置。
商城已移除 setnftcaps 和 setxferwl；NFT 白名单请直接通过底层 NFT 合约管理。
若从统一白名单版升级，仍需重新调用 NFT 合约的 setcreator 清理旧模式标记。完整操作见 [mall.nft 使用说明](../mall.nft/README.md)。

## 1. 调用约定

示例合约账户为 `mall.cisum`，普通销售示例的 NFT 合约为原版 `ticket.cvnft`（setsupply 等扩展方法须使用 mall.nft 等支持对应接口的后端），支付合约为 `cisum.token`，币种精度为 `4,CISUM`。账户必须预先存在，钱包需具备对应签名权限。示例不指定节点地址，调用时请通过 `fucli -u <节点地址>` 选择环境。

示例是各接口的独立用法，不应不加检查地从头连续执行；商品 ID、操作号、库存、上架状态和预算应以实际链上数据为准。文中 `product_id=1` 假设为新建商品，`product_id=2` 假设为库存商品。

### 1.1 标识与数量

| 名称 | 含义 |
|---|---|
| `product_id` | 商城商品编号，首次一般为 1；购买 memo 使用此编号 |
| `nft_id` | 底层 NFT 编号；新建从 6000000001 递增，已有 NFT 保留原编号 |
| `maximum` | NFT 历史累计发行上限，不是当前可售库存 |
| `quota` | 商城累计销售配额，**设置绝对值，不是增加值** |
| `sold` | 商城累计已售数量 |
| `quota - sold` | 剩余销售配额；库存模式还受真实余额约束 |

同一 NFT 以 `(NFT 合约账户, nft_id)` 唯一识别。`uint64` 标识在 JavaScript 中建议用十进制字符串或 BigInt 处理，避免未来超过安全整数范围；示例使用当前可精确表示的数字。

### 1.2 通用管理参数

下文标注“通用管理参数”的接口都包含以下两个参数，不可省略：

| 参数 | 类型 | 含义 |
|---|---|---|
| `op_id` | `uint64` | 大于 0 的管理操作号，在整个商城 `operations` 表中唯一，跨接口、跨管理员共用命名空间 |
| `reason` | `string` | 操作原因，长度 1–256 字节；中文一个字符通常占多个字节 |

重复 `op_id` 会拒绝交易，不会返回此前结果。广播超时应先查操作记录；失败交易不占用操作号。初始化、登记资产合约、空投、成交通知不使用这两个参数；空投有独立的 `batch_id`。

### 1.3 `sale_terms` 销售参数

| 字段 | 类型 | 含义与约束 |
|---|---|---|
| `price` | `asset` | 单份价格，如 `10.0000 CISUM`，必须大于 0，币种及精度须已登记 |
| `starts` | `time_point_sec` | 开售时间，UTC，秒级，必须大于 Unix 时间 0；达到该时间即可购买 |
| `per_purchase` | `uint32` | 单次购买上限，必须大于 0 |
| `per_account` | `uint32` | 每账户对该 NFT 的累计购买上限，不得小于单次上限；0 不表示无限制 |

例如 `2026-11-10T00:00:00` 表示北京时间 2026-11-10 08:00:00。历史时间可用于立即开售，但商品仍需上架。当前没有结束时间字段。

累计购买量只计成功购买；转出、销毁、下架重上架均不清零，接收他人赠送不计入购买量。

## 2. 初始化与资产登记

### 2.1 `init`：初始化或更换超级管理员

**权限：商城合约账户。** 唯一参数 admin（name）必须是已存在账户，写入 authconfig。重复调用不会重置商品、配额、成交、资金、NFT 编号或暂停状态。

```sh
fucli push action mall.cisum init '{"admin":"manager"}' -p mall.cisum@active
```

业务授权与 show.cisum 的 require_perm 模式一致：合约自身授权可执行；submitter 为超级管理员并有其授权时可执行；其他账户必须签名，并通过 cisum.auth::checkrole 检查相应权限。合约自身调用时日志 actor 记录合约账户。超级管理员不等同于 cisum.auth 的管理员。

| 权限字符串 | 对应方法 |
|---|---|
| productMgmt | addbank、advancenft、createnft、regstock、setsale、setquota、setsupply、setmeta、setstatus、withdrawnft |
| mallTreasury | allocfund |
| mallAirdrop | airdrop |
| mallPause | pause |

以上业务方法的第一个参数均新增 submitter（name），表示操作账户。init、notifybuy 仍仅允许商城自身授权。用户直接转账购买不需要上述管理权限；空投收款人也不需要管理角色。商城不将 cisum.auth.allowlist 作为业务权限的替代条件。

角色和成员统一在 cisum.auth 管理。例如由其管理员 authadmin 授权 alice 管理商品：

```sh
fucli push action cisum.auth addrole '{"submitter":"authadmin","role":"mallmanager","desc":"商城商品管理"}' -p authadmin@active
fucli push action cisum.auth addroleperm '{"submitter":"authadmin","role":"mallmanager","perms":["productMgmt"],"desc":"商品管理权限"}' -p authadmin@active
fucli push action cisum.auth grantrole '{"granter":"authadmin","user":"alice","role":"mallmanager"}' -p authadmin@active
fucli push action cisum.auth revokerole '{"granter":"authadmin","user":"alice","role":"mallmanager"}' -p authadmin@active
```

角色名称可自定义，权限字符串大小写必须一致。商城需要自身 active 的代码执行权限以发送 checkrole 内联动作；校验失败会使整笔业务交易回滚。

### 2.2 `addbank`：登记支持的资产合约

**权限：合约自身、超级管理员或拥有 `productMgmt` 权限的 submitter。** 登记支付/空投代币，或经验证的 NFT 合约。

| 参数 | 类型 | 意义 |
|---|---|---|
| `submitter` | `name` | 操作账户，须提供其签名并具有 productMgmt 权限；超级管理员可直接授权 |
| `bank` | `name` | 实际发行及转账合约，必须存在且不能是商城自身 |
| `nft` | `bool` | `true` 为 NFT，`false` 为普通代币 |
| `sym` | `symbol` | 普通代币完整符号，格式“精度,币种”；NFT 模式忽略其业务意义，但 ABI 参数仍需传入 |

登记 CISUM 支付币种及 NFT 合约：

```sh
fucli push action mall.cisum addbank '{"submitter":"manager","bank":"cisum.token","nft":false,"sym":"4,CISUM"}' -p manager@active
fucli push action mall.cisum addbank '{"submitter":"manager","bank":"ticket.cvnft","nft":true,"sym":"4,CISUM"}' -p manager@active
```

同一普通代币合约可登记多个不同符号；重复登记同一符号会拒绝。同一合约不能切换普通代币/NFT 类型。当前无删除登记接口，也不会自动验证第三方合约 ABI，必须先人工确认兼容性。

### 2.3 `advancenft`：向前调整新建 NFT 编号游标

**权限：合约自身、超级管理员或拥有 `productMgmt` 权限的 submitter。** 正常创建会自动分配编号，不需要调用本方法。连续遇到 100 个已占用编号导致创建失败时，可核对后推进游标。

| 参数 | 类型 | 意义 |
|---|---|---|
| `next_id` | `uint64` | 下一次尝试分配的编号，必须大于当前游标且小于 `uint64` 最大值 |
| `op_id`、`reason` | 见通用参数 | 操作号及推进原因 |

```sh
fucli push action mall.cisum advancenft '{"submitter":"manager","next_id":6000001001,"op_id":1001,"reason":"已核对前段编号被占用"}' -p manager@active
```

只调整后续新建编号，不修改已创建或已接入 NFT；不能向后回拨。执行此示例后下一次新建将不再从 6000000001 开始。

## 3. NFT 创建与接入

### 3.1 `createnft`：创建按需铸造商品

**权限：合约自身、超级管理员或拥有 `productMgmt` 权限的 submitter，且商城须具备底层创建及内联执行权限。** 自动创建 NFT 模板、登记下架商品并记录商城销售配额；不调用 setpolicy/setxferwl，不检查或配置发送方白名单。创建时不铸造全部数量，购买时才铸造。

| 参数 | 类型 | 意义 |
|---|---|---|
| `product_code` | `string` | 业务商品编号，必填，1–256 字节，原样保存；不校验唯一性 |
| `bank` | `name` | 已登记的兼容 NFT 合约 |
| `uri` | `string` | NFT 元数据地址；当前底层要求少于 1024 字节且创建时不能与已有 URI 重复 |
| `ipowner` | `name` | 知识产权归属账户，传存在的账户；底层也允许空账户名 |
| `maximum` | `int64` | 最大发行量，正整数，不超过 `2^62 - 1` |
| `quota` | `int64` | 初始累计销售配额，范围 0 至 maximum |
| `paybank` | `name` | 已登记的支付代币合约 |
| `terms` | `sale_terms` | 单价、开售时间及限购设置，见 1.3 |
| `op_id`、`reason` | 见通用参数 | 操作号及创建原因 |

```sh
fucli push action mall.cisum createnft '{"submitter":"manager","product_code":"SKU-001","bank":"ticket.cvnft","uri":"ipfs://mall-badge-001","ipowner":"manager","maximum":1000,"quota":200,"paybank":"cisum.token","terms":{"price":"10.0000 CISUM","starts":"2026-11-10T00:00:00","per_purchase":5,"per_account":20},"op_id":1002,"reason":"创建首批商城勋章"}' -p manager@active
```

在全新商城、编号未占用且未推进游标时，得到 `product_id=1`、`nft_id=6000000001`、`mint=true`、`status=0`。以 products 查询结果为准；任何内联步骤失败，编号及商品登记一起回滚。

原版 ticket.cvnft 可由合约账户配置全局白名单，例如：

```sh
fucli push action ticket.cvnft addwhitelist '{"acct":"mall.cisum"}' -p ticket.cvnft@active
```

商城不调用 setpolicy 或 setreserve，也不检查策略表。上架成功不代表已经具备发货权限，实际转出由 NFT 合约检查。mall.nft 应独立配置逐 NFT 发送白名单。

发行空间有 issuance 记录时使用累计 minted，没有时使用当前 supply（原版 ticket.cvnft 的规则，销毁可恢复发行空间）。setsupply 仍需 issuance/setmaxsup 扩展，原版 ticket.cvnft 不支持；setmeta 仍须符合底层授权规则。

### 3.2 `regstock`：接入已有 NFT 库存

**权限：合约自身、超级管理员或拥有 `productMgmt` 权限的 submitter。** 保留原 NFT ID，登记 `mint=false` 的下架商品，不创建模板、不铸造。

| 参数 | 类型 | 意义 |
|---|---|---|
| `bank` | `name` | 已登记的兼容 NFT 合约 |
| `nft_id` | `uint64` | 已存在 NFT 的原始编号 |
| `quota` | `int64` | 初始累计配额；不能超过商城可用余额，可设为 0 后再补货 |
| `paybank` | `name` | 支付代币合约 |
| `terms` | `sale_terms` | 销售设置，见 1.3 |
| `op_id`、`reason` | 见通用参数 | 操作号及接入原因 |

假设商城已持有 NFT 42 共 100 份：

```sh
fucli push action mall.cisum regstock '{"submitter":"manager","bank":"ticket.cvnft","nft_id":42,"quota":100,"paybank":"cisum.token","terms":{"price":"10.0000 CISUM","starts":"2026-11-10T00:00:00","per_purchase":5,"per_account":20},"op_id":1003,"reason":"接入现有库存"}' -p manager@active
```

同一 `(bank, nft_id)` 不能重复登记。登记不检查策略或白名单；发货时由底层检查转出权限。

## 4. 销售与资产管理

### 4.1 `setsale`：修改销售参数

**权限：合约自身、超级管理员或拥有 `productMgmt` 权限的 submitter。必须处于下架状态 0。** 整体替换销售参数，不更换支付合约。

| 参数 | 类型 | 意义 |
|---|---|---|
| `product_id` | `uint64` | 目标商品编号 |
| `terms` | `sale_terms` | 完整新设置，四个字段均需提供 |
| `op_id`、`reason` | 见通用参数 | 操作号及修改原因 |

```sh
fucli push action mall.cisum setsale '{"submitter":"manager","product_id":1,"terms":{"price":"12.0000 CISUM","starts":"2026-12-01T00:00:00","per_purchase":3,"per_account":10},"op_id":1004,"reason":"调整下一期销售价格及限购"}' -p manager@active
```

不会清零历史购买量。若新累计上限低于某账户已购量，该账户后续购买会被拒绝。

### 4.2 `setquota`：调整累计销售配额

**权限：合约自身、超级管理员或拥有 `productMgmt` 权限的 submitter。** 下架、上架状态都可修改，归档商品不可修改。

| 参数 | 类型 | 意义 |
|---|---|---|
| `product_id` | `uint64` | 商品编号 |
| `quota` | `int64` | 新累计配额，非负且不低于 sold，不超过 `2^62 - 1` |
| `op_id`、`reason` | 见通用参数 | 操作号及原因 |

```sh
fucli push action mall.cisum setquota '{"submitter":"manager","product_id":1,"quota":300,"op_id":1005,"reason":"追加可售配额"}' -p manager@active
```

若已售 50，设置为 300 后剩余配额是 250。按需铸造模式须有足够剩余发行空间，不调用底层 setreserve；库存模式须有足够真实余额。

### 4.3 `setsupply`：调整发行上限及配额

**权限：合约自身、超级管理员或拥有 `productMgmt` 权限的 submitter，且底层 NFT 的 issuer 必须是商城。商品必须下架。** 同一交易内更新发行上限与商城配额。

| 参数 | 类型 | 意义 |
|---|---|---|
| `product_id` | `uint64` | 商品编号 |
| `maximum` | `int64` | 新发行上限，正整数，不低于历史累计铸造量 |
| `quota` | `int64` | 同时设置的新累计销售配额，不低于已售数量 |
| `op_id`、`reason` | 见通用参数 | 操作号及原因 |

```sh
fucli push action mall.cisum setsupply '{"submitter":"manager","product_id":1,"maximum":2000,"quota":500,"op_id":1006,"reason":"扩大发行上限及商城配额"}' -p manager@active
```

按需铸造模式必须满足 `累计铸造量 + quota - sold <= maximum`。库存模式仍需检查实际余额及底层已有预留。旧 NFT 必须先核验历史发行量；当前接口不支持仅凭外部发行者联合签名修改非商城发行的 NFT。

### 4.4 `setmeta`：修改元数据地址

**权限：合约自身、超级管理员或拥有 `productMgmt` 权限的 submitter；商品下架，且 NFT 发行者为商城。** 调用底层 settokenuri 更新 URI，不负责上传文件。

| 参数 | 类型 | 意义 |
|---|---|---|
| `product_id` | `uint64` | 商品编号 |
| `uri` | `string` | 新元数据地址，当前底层要求少于 1024 字节且不能与其他 NFT URI 重复 |
| `op_id`、`reason` | 见通用参数 | 操作号及原因 |

```sh
fucli push action mall.cisum setmeta '{"submitter":"manager","product_id":1,"uri":"ipfs://mall-badge-001-v2","op_id":1007,"reason":"更新勋章图片与说明"}' -p manager@active
```

URI 存在底层 tokenstats 中，商城 products 不重复保存。已有库存 NFT 若由其他账户发行，应由有权账户通过底层接口维护。

### 4.5 `setstatus`：上架、下架或永久归档

**权限：合约自身、超级管理员或拥有 `productMgmt` 权限的 submitter。**

| 参数 | 类型 | 意义 |
|---|---|---|
| `product_id` | `uint64` | 商品编号 |
| `status` | `uint8` | 0 下架、1 上架、2 永久归档 |
| `op_id`、`reason` | 见通用参数 | 操作号及原因 |

```sh
fucli push action mall.cisum setstatus '{"submitter":"manager","product_id":1,"status":1,"op_id":1008,"reason":"商品上架"}' -p manager@active
```

上架时校验库存/发行空间，不预检查转出资格，可以提前上架，但到 starts 才允许购买。下架不会清空数据。归档需先令 quota=sold，归档后不能恢复状态；归档记录及累计限购记录不删除。

### 4.6 `withdrawnft`：取回未占用的 NFT 库存

**权限：合约自身、超级管理员或拥有 `productMgmt` 权限的 submitter。仅适用于下架的库存商品，不能用于按需铸造商品。**

| 参数 | 类型 | 意义 |
|---|---|---|
| `product_id` | `uint64` | 库存商品编号 |
| `to` | `name` | 接收账户，须存在且不是商城自身 |
| `quantity` | `int64` | 取回份数，正整数，不超过 `真实余额 - (quota - sold)` |
| `op_id`、`reason` | 见通用参数 | 操作号及原因 |

假设商品 2 已下架，商城余额 100，剩余配额 80：

```sh
fucli push action mall.cisum withdrawnft '{"submitter":"manager","product_id":2,"to":"manager","quantity":20,"op_id":1010,"reason":"取回未分配库存"}' -p manager@active
```

仍受底层发送方白名单约束。需要提取更多时，先通过 setquota 释放配额。

## 5. 资金、空投与暂停

### 5.1 `allocfund`：销售收入划为空投预算

**权限：合约自身、超级管理员或拥有 `mallTreasury` 权限的 submitter。** 仅进行内部记账，资金仍留在商城账户，不发送代币。

| 参数 | 类型 | 意义 |
|---|---|---|
| `bank` | `name` | 已登记的普通代币合约 |
| `quantity` | `asset` | 划拨金额，必须为正，币种及精度已登记，不能超过 sales |
| `op_id`、`reason` | 见通用参数 | 操作号及原因 |

```sh
fucli push action mall.cisum allocfund '{"submitter":"treasurer","bank":"cisum.token","quantity":"100.0000 CISUM","op_id":1011,"reason":"划拨活动空投预算"}' -p treasurer@active
```

成功后 sales 减少、drops 等额增加。也可直接转账充值 drops，见 6.2。当前无反向划拨或销售收入提现接口。

### 5.2 `airdrop`：批量空投普通代币

**权限：合约自身、超级管理员或拥有 `mallAirdrop` 权限的 submitter，空投未暂停。** 从 drops 扣预算并执行内联转账，任一条失败整批回滚。

| 参数 | 类型 | 意义 |
|---|---|---|
| `batch_id` | `uint64` | 大于 0 的批次号，整个商城唯一，跨币种及代币合约去重 |
| `bank` | `name` | 已登记的普通代币合约 |
| `entries` | `payment[]` | 1–50 个收款条目，金额币种与精度必须完全相同 |
| `entries[].to` | `name` | 收款账户，必须存在且不是商城自身 |
| `entries[].quantity` | `asset` | 正数金额，合计不能超过 drops |

```sh
fucli push action mall.cisum airdrop '{"submitter":"distributor","batch_id":2001,"bank":"cisum.token","entries":[{"to":"alice","quantity":"1.0000 CISUM"},{"to":"bob","quantity":"2.0000 CISUM"}]}' -p distributor@active
```

此例扣除 3 CISUM 空投预算。成功批次不可重复；失败批次回滚，可修正后重试原批次号。允许同一账户出现在多条记录中，会按各条分别发放；不会自动合并或去重收款人。它不铸造代币，也不发 NFT。

### 5.3 `pause`：暂停或恢复业务

**权限：合约自身、超级管理员或拥有 `mallPause` 权限的 submitter。** 同时写入两个暂停标志。

| 参数 | 类型 | 意义 |
|---|---|---|
| `buying` | `bool` | true 暂停购买；false 恢复购买 |
| `dropping` | `bool` | true 暂停空投；false 恢复空投 |
| `op_id`、`reason` | 见通用参数 | 操作号及原因 |

```sh
fucli push action mall.cisum pause '{"submitter":"guardian","buying":true,"dropping":false,"op_id":1012,"reason":"暂停购买进行维护，保留空投"}' -p guardian@active
```

注意 true 表示暂停，不是启用。恢复两项时均传 false。暂停不阻止商品管理、资金划拨、空投充值或库存补货，也不改变底层 NFT 持有人转账权限。

## 6. 用户转账与通知入口

### 6.1 `ontransfer`：监听转账并完成购买

这是 `*::transfer` 通知处理器，**不是可直接调用的 action，没有独立 ABI 参数**。根据实际发出通知的已登记合约，解析普通代币或 NFT 转账。

用户购买时调用支付代币合约的 transfer：

| 参数 | 类型 | 意义 |
|---|---|---|
| `from` | `name` | 付款账户，须签名，也是 NFT 接收账户 |
| `to` | `name` | 商城账户，此处为 mall.cisum |
| `quantity` | `asset` | 实际支付金额，等于单价乘以预期份数 |
| `memo` | `string` | `mall:v1:<product_id>:<expected_quantity>:<request_id>` |

memo 字段：

| 字段 | 意义 |
|---|---|
| `mall:v1` | 固定协议前缀和版本 |
| `product_id` | 正整数商城商品编号，**不是 nft_id** |
| `expected_quantity` | 预期购买份数，正整数；必须等于金额除以当前单价 |
| `request_id` | 1–64 位 ASCII 字母、数字、`-`、`_`；仅作业务关联，链上不校验唯一性 |

假设商品 1 已完成转出授权、已到开售时间、单价为 10 CISUM，购买 2 份：

```sh
fucli push action cisum.token transfer '{"from":"alice","to":"mall.cisum","quantity":"20.0000 CISUM","memo":"mall:v1:1:2:order-20261110-001"}' -p alice@active
```

成功时 alice 收到 2 份 NFT，20 CISUM 保留在商城并记入 sales。商品和账户计数同步增加，发出 notifybuy。没有独立 purchase action，无需给商城代扣授权。

必须满足上架、到开售时间、未暂停、限购、配额和供货条件。金额不能有余数，不自动退款找零；任一步失败，原付款及相关链上状态一起回滚。重复 request_id 仍可购买，不提供请求号幂等保障。

### 6.2 普通代币充值空投预算

同样调用代币 transfer，`from/to/quantity` 含义同上，但 memo 固定为 `mallfund:v1`，数量为充值金额，**不购买 NFT**。充值不限制为管理员，付款人需授权。

```sh
fucli push action cisum.token transfer '{"from":"manager","to":"mall.cisum","quantity":"100.0000 CISUM","memo":"mallfund:v1"}' -p manager@active
```

只增加对应 drops。充值没有 request_id，每次成功转账都是一次新充值。

### 6.3 NFT 库存补货

调用已登记 NFT 合约的 transfer：

| 参数 | 类型 | 意义 |
|---|---|---|
| `from` | `name` | 库存提供方，须签名且在每种 NFT 的转出白名单中 |
| `to` | `name` | 商城账户 |
| `assets` | `nasset[]` | 1–50 个 NFT 数量条目，逐项校验 |
| `assets[].amount` | `int64` | 正数份数 |
| `assets[].symbol.nid` | `uint64` | 底层 NFT ID，不是商品 ID |
| `memo` | `string` | 固定为 `mallstock:v1` |

假设 NFT 42 已通过 regstock 登记成未归档的库存商品，商城与 manager 均有转出资格：

```sh
fucli push action ticket.cvnft transfer '{"from":"manager","to":"mall.cisum","assets":[{"amount":50,"symbol":{"nid":42}}],"memo":"mallstock:v1"}' -p manager@active
```

补货只增加真实余额，不自动提高 quota，之后按需调用 setquota。不接受按需铸造商品作为补货目标。多项中任一项无效会导致整笔回滚。

### 6.4 `notifybuy`：内部成交通知

**权限：商城合约账户。** 成功购买流程自动发出的内联 action，供后端扫描，不负责扣款、发 NFT 或修改记录。不要把手工调用它当成购买入口，也不能仅凭这条通知判定一笔付款已经发生，应核对同笔交易中的实际付款和 NFT 发放。

| 参数 | 类型 | 意义 |
|---|---|---|
| `buyer` | `name` | 实际购买账户 |
| `request_id` | `string` | 购买请求号 |
| `product_id` | `uint64` | 商城商品编号 |
| `bank` | `name` | **NFT 合约**，不是支付代币合约 |
| `nft_id` | `uint64` | 发放的底层 NFT ID |
| `mint` | `bool` | true 按需铸造，false 已有库存转出 |
| `quantity` | `int64` | 成交份数 |
| `paid` | `asset` | 实付金额 |

上述购买可能产生如下 action data 示例（自动生成，无需手工 push）：

```json
{"buyer":"alice","request_id":"order-20261110-001","product_id":1,"bank":"ticket.cvnft","nft_id":6000000001,"mint":true,"quantity":2,"paid":"20.0000 CISUM"}
```

支付合约从 products.pay_contract 或原始支付 action 获取。此方法只生成 action trace，不通过 require_recipient 主动通知买家的合约。

## 7. 查询与对账

所有查询合约为 mall.cisum：

| 表 | scope | 用途 |
|---|---|---|
| `authconfig` | mall.cisum | 统一授权的超级管理员 admin |
| `config` | mall.cisum | 历史角色字段（不再授权）、编号游标、暂停状态 |
| `banks` | mall.cisum | 已登记的资产合约及类型 |
| `products` | mall.cisum | 商品、销售参数、供货模式和数量 |
| `funds` | 代币合约，例如 cisum.token | sales 与 drops，按完整 symbol 分账 |
| `operations` | mall.cisum | 管理操作号、操作人、原因及变更快照 |
| `batches` | mall.cisum | 空投批次号、代币合约、总额和内容摘要 |
| `purchases` | 买家，例如 alice | 该账户各商品的历史累计购买量 |

```sh
fucli get table mall.cisum mall.cisum products --limit 100
fucli get table mall.cisum cisum.token funds
fucli get table mall.cisum alice purchases
fucli get table mall.cisum mall.cisum operations --lower 1002 --upper 1003
```

funds.sales/drops 使用最小单位整数；对于 4 位小数 CISUM，10000 表示 1 CISUM。operations.before/after 为按相应操作数据结构序列化的二进制快照，不是 JSON 文本。

查询超过单页时须按节点返回的 more/next_key 继续分页。核销订单应验证买家、商品、支付合约、金额及数量，并等待链上最终性；不要仅凭 request_id 或通知内容匹配订单。

## 8. 常见失败原因

| 错误前缀 | 含义及处理 |
|---|---|
| `MALL_CONFIG` | 未初始化或管理员账户无效 |
| `MALL_BANK` | 合约/符号未登记，或资产类型不符 |
| `MALL_STATUS` | 上下架或归档状态不满足操作条件 |
| `MALL_TIME` | 尚未到开售时间或时间参数无效 |
| `MALL_LIMIT` | 超出单次/累计限购，或限购参数无效 |
| `MALL_PAYMENT` | 支付币种不符、非整数倍金额或预期数量不符 |
| `MALL_MEMO` / `MALL_REQUEST` | memo 格式错误、非法请求号 |
| `MALL_QUOTA` / `MALL_STOCK` | 配额或真实库存不足，或库存状态不符 |
| `MALL_SUPPLY` / `MALL_META` | 发行权限、发行量或元数据操作不满足条件 |
| `MALL_FUNDS` / `MALL_BATCH` | 预算不足、批次无效或重复 |
| `MALL_OPERATION` / `MALL_ID` | 管理操作号重复、原因无效或编号需要处理 |
| `MALL_PAUSED` | 对应业务已暂停 |
| `MALL_AMOUNT` / `MALL_PRICE` / `MALL_NFT` | 金额、价格或 NFT/商品参数无效 |

权限缺失以及底层 NFT/代币合约错误可能返回不同格式，不一定带 MALL 前缀。失败不会扣留本次支付，但链上执行资源消耗应按目标链规则另行处理。


## 数据升级与编号说明

products 新增的 product_code 是字符串业务编号；原 id 仍是 uint64 主键。购买 memo、其他管理方法的 product_id 参数、purchases/notifybuy 内的 product_id 仍指数字主键，不改为字符串。例如业务编号 SKU-001 的商品若 id=1，购买 memo 仍为 mall:v1:1:2:order1。

createnft 的 product_code 参数位于参数列表末尾，JSON 对象可按字段名传入；旧客户端必须增加此参数。regstock 接入的库存商品保存空字符串，当前没有单独修改业务编号的接口。

为兼容旧链上数据，products.product_code 在 ABI 中为 string$（尾部 binary_extension），旧行可能缺少此字段。config 保留旧四个 name 和 roles 数组的布局，但它们不再参与授权。超级管理员存储在新的 authconfig 表中。

升级必须由商城自身调用 init(admin) 后再恢复普通管理调用，并在 cisum.auth 中配置角色；旧数组不会自动迁移，也不会将旧 manager 自动提升为超级管理员。升级前已有的商品、购买计数、回执和资金均保留。客户端须同步新 ABI：init 改为单个 admin，13 个业务方法增加首参数 submitter，删除本地 addrole/delrole。

operations 的 before/after 是当时版本的序列化快照，历史记录需按对应版本结构解码。

接口移除：新版 ABI 不再包含 setnftcaps、setxferwl 和 nftcaps 表定义。已有 nftcaps 链上数据不会自动删除，但商城不再读写；旧客户端必须停止调用这两个方法。底层 mall.nft 的 setxferwl 保持可用，请直接在 NFT 合约上配置权限。

业务编号：products 的字符串 product_id 已更名为 product_code，createnft 参数同步更名。purchases 在尾部追加 product_code（string$），保存业务编号；数字 product_id 保持原意。旧商品字符串值无需迁移；旧购买累计行仍可读取。再次购买时累计行会补齐业务编号。无业务编号的旧商品及库存商品记录空字符串。

回执表已移除：不再写入 receipts，也不按 request_id 去重。相同请求编号再次付款会按新购买处理，仍受限购和配额检查；request_id 保留格式校验及通知用途。后端应根据交易 ID 和实际成交轨迹确认结果，避免盲目重付。升级不会自动删除历史 receipts 行或返还其 RAM，本次节省的是后续回执及索引写入资源。
