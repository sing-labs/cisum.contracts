# mall.nft：逐 NFT 转出白名单

每一种 NFT 都有独立的发送方白名单。允许账户转出 NFT A，不代表它能转出 NFT B。
创建 NFT 的授权名单保持独立；空的创建名单拒绝所有创建。

## 权限与行为

- `setcreator` 只能由 NFT 合约账户调用，指定哪些 issuer 可以创建模板。创建时必须由 issuer 签名；移除创建资格不撤销其已有 NFT 的发行者身份。
- 所有 NFT 从创建后就检查自身发送方白名单；接收方无需白名单，可以接收、购买、持有。接收方在名单或合约额外签名不能绕过发送方检查。
- `setpolicy(nft_id, manager)` 由 NFT 发行者调用，为该 NFT 指定一个额外的白名单管理员。每个 NFT 只能设置一次，不可修改或删除。
- `setpolicy` 不自动加入任何白名单，也不授予铸造权限。发行者与指定 manager 都可管理该 NFT 的白名单。
- `setxferwl(nft_id, account, allowed)` 按 NFT 加入/移除发送方。发行者在未指定 manager 前也可管理；manager 仅能管理被委托的 NFT。
- 发行者和商城也需要对应发送权限才能转出。批量转账逐项检查，任一项失败整笔回滚。

本合约与 ticket.cvnft 的区别：mall.nft 即使尚未设置 policy，也不会走旧全局白名单放行逻辑。

## 编译与测试

```sh
bash tests/mall.cisum/build.sh
python3 tests/mall.nft/integration.py
python3 tests/mall.cisum/integration.py
```

Docker 镜像为 floncdt:v0.8.1，产物为 `build/mall-verified/mall.nft.wasm` 和 `mall.nft.abi`。
本地链测试验证创建权限、逐 NFT 白名单隔离、管理员权限、批量回滚与商城购买。

升级回归测试可使用保存的上一版统一白名单 WASM/ABI（文件名为 `build/mall-verified/global.mall.nft.wasm` 与 `.abi`）运行：

```sh
python3 tests/mall.nft/integration.py --global-fixture
```

这些历史二进制仅为本地测试夹具，不提交仓库、不用于部署。`version_compat.py` 为此前 setpolicy 版本错配的历史复现工具，明确使用这套统一白名单夹具。

## 商城接入

部署 NFT 合约，授权商城创建，并在商城登记资产合约：

```sh
fucli set contract mall.nft build/mall-verified mall.nft.wasm mall.nft.abi -p mall.nft@active
fucli push action mall.nft setcreator '{"creator":"mall.cisum","to_add":true}' -p mall.nft@active
fucli push action mall.cisum addbank '{"bank":"mall.nft","nft":true,"sym":"4,CISUM"}' -p mall.cisum@active
```

商城还须完成 init、支付币种登记和自身内联执行授权。调用 createnft 时使用 `bank=mall.nft`：

```sh
fucli push action mall.cisum createnft '{"bank":"mall.nft","uri":"ipfs://mall-nft-001","ipowner":"manager","maximum":1000,"quota":200,"paybank":"cisum.token","terms":{"price":"10.0000 CISUM","starts":"2026-11-10T00:00:00","per_purchase":5,"per_account":20},"op_id":3001,"reason":"创建商城 NFT"}' -p manager@active
```

商城创建流程不再调用 setpolicy/setxferwl，也不需要配置 setnftcaps。创建成功后，需独立配置发送权限才能成功发货（商城上架不预检查权限）：

```sh
# 假设新 NFT 编号为 6000000001，发行者是 mall.cisum。
fucli push action mall.nft setpolicy '{"nft_id":6000000001,"manager":"manager"}' -p mall.cisum@active
fucli push action mall.nft setxferwl '{"nft_id":6000000001,"account":"mall.cisum","allowed":true}' -p manager@active
```

这两步与创建是独立操作；不配置时创建仍成功，上架不受影响，但购买发货会被 mall.nft 拒绝，整笔交易回滚。

编号由商城游标分配（初始 6000000001）。直接调用底层 create 则由授权创建者传入非零唯一编号。

## 方法与参数

| 方法 | 权限 | 参数及作用 |
|---|---|---|
| `setcreator` | NFT 合约账户 | `creator:name` 创建账户；`to_add:bool` true 授权、false 移除。重复设置安全，并清理旧统一模式标记 |
| `setpolicy` | 该 NFT 发行者 | `nft_id:uint64` NFT 编号；`manager:name` 现有账户；指定一次性白名单管理员，不添加发送方 |
| `setxferwl` | 该 NFT 发行者或其 manager | `nft_id:uint64`；`account:name` 现有账户；`allowed:bool` 加入/移除该 NFT 发送白名单 |
| `create` | 授权名单中的 issuer | `issuer:name`；`maximum_supply:int64` 正数历史发行上限；`symbol:nsymbol` 如 `{"nid":6000000001}`；`token_uri:string` 唯一 URI，少于 1024 字节；`ipowner:name` 存在的版权账户或空名称。仅创建模板 |
| `issue` | NFT 发行者 | `to:name` 必须等于发行者；`quantity:nasset` 正数份数；`memo:string` 最多 256 字节。铸造到发行者 |
| `retire` | NFT 发行者 | `quantity:nasset` 销毁自己持有的正数份数；`memo:string` 最多 256 字节。不恢复历史发行额度 |
| `transfer` | 发送方 | `from:name`；`to:name` 存在且不同于发送方；`assets:nasset[]` 1–50 项正数数量；`memo:string` 最多 256 字节。每项都检查对应 NFT 的发送权限 |
| `setmaxsup` | NFT 发行者 | `nft_id:uint64`；`maximum:int64` 新上限，必须为正且不低于累计铸造量加预留量 |
| `setreserve` | NFT 发行者 | `nft_id:uint64`；`reserved:int64` 非负发行预留，不超出剩余空间 |
| `settokenuri` | NFT 发行者或合约账户 | `symbid:uint64` NFT ID；`url:string` 新 URI，少于 1024 字节，不能与其他 NFT 重复 |

`nasset` 示例：`{"amount":2,"symbol":{"nid":6000000001}}`；份数是整数，不超过 `2^62 - 1`。

## 独立设置示例

假设 NFT 6000000100 已由 issuer 创建：

```sh
# 指定 manager 管理这一种 NFT 的白名单；需要 issuer 签名。
fucli push action mall.nft setpolicy '{"nft_id":6000000100,"manager":"manager"}' -p issuer@active

# 允许 alice 转出这一种 NFT，不影响其他 NFT。
fucli push action mall.nft setxferwl '{"nft_id":6000000100,"account":"alice","allowed":true}' -p manager@active
fucli push action mall.nft transfer '{"from":"alice","to":"bob","assets":[{"amount":1,"symbol":{"nid":6000000100}}],"memo":"授权转出"}' -p alice@active

# 撤销这一种 NFT 的发送资格。
fucli push action mall.nft setxferwl '{"nft_id":6000000100,"account":"alice","allowed":false}' -p manager@active

# 查询单个 NFT 的白名单和所有 NFT 的 manager 配置。
fucli get table mall.nft 6000000100 xferwhite
fucli get table mall.nft mall.nft xferpolicy
```

## 从统一白名单版升级

更新代码不会自动清理链上表。部署新版前暂停购买，然后：

1. 部署新的 mall.nft WASM 和 ABI。
2. 由 NFT 合约账户重新调用一次 `setcreator`（例如继续授权商城）。该动作会清除旧 `xfermode` 标记，避免商城误判为统一模式。
3. 确认商城部署了创建与权限分离版本；无需设置 setnftcaps。
4. 对已有商品，发行者逐个调用 setpolicy，并为商城、补货方等设置对应 NFT 的 setxferwl。已有 policy 如存在则沿用，不重复调用。
5. 确认逐 NFT 授权后恢复购买。

旧 `senders` 统一名单不再用于任何转账校验，也不会自动复制到每种 NFT。若更早版本遗留了 xferwhite/xferpolicy，则这些逐 NFT 记录会继续有效，应逐项核对。
余额、发行历史、创建授权名单保留。ABI 中保留 senders/xfermode 的定义用于旧数据查询和兼容清理，它们不是当前有效的授权来源。

仓库只更新代码和本地编译产物，不会自动修改链上部署。
