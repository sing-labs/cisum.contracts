# mall.cisum

完整方法用途、参数和调用示例见 [合约接口说明](API.md)。
新增配套底层合约见 [mall.nft](../mall.nft/README.md)，默认强制创建授权及逐 NFT 转出白名单。

多 NFT 商城：转账通知内完成支付、NFT 发放与计数，失败整笔回滚。
支持 mall.nft 和原版 ticket.cvnft 的创建、上架、购买及库存销售；不预检查策略表或转出白名单。转出权限由底层 NFT 合约执行，失败整笔交易回滚。
货款保留在商城，不转入 cisum.vault。仅支持已验证的 `ticket.cvnft` ABI 兼容 NFT 合约；
不要把具有不同余额表或 transfer 参数的第三方 NFT 合约加入白名单。

## 构建与本地验证

```sh
bash tests/mall.cisum/build.sh
python3 tests/mall.cisum/integration.py
python3 tests/mall.nft/integration.py
```

构建使用本地 `floncdt:v0.8.1`，输出至 `build/mall-verified`。
集成测试使用 `fucli:v0.8.3` 启动无外网、无宿主端口映射的一次性链，测试后销毁。
不接触生产节点或宿主钱包。测试创世密钥是公开开发密钥，禁止用于正式部署。

## 部署顺序

1. 准备兼容的 NFT 合约，例如原版 ticket.cvnft 或 mall.nft。
2. 部署商城，为其 active 权限配置自身 `eosio.code` 执行权限；实际网络的权限名称
   以 `fucli set account permission ... --add-code` 生成结果为准。
3. init(admin) 由合约自身指定超级管理员，其他业务账户通过 cisum.auth 的角色权限管理。
4. `addbank(submitter, bank, nft, sym)` 由具有 productMgmt 权限的账户登记经过验证的合约。普通代币逐个登记完整 symbol，
   如 `4,CISUM`；NFT 的 sym 参数忽略。一个地址不能同时登记两种资产类型。
5. 根据底层配置给予商城创建许可、必要的 DID 以及支付代币的转账许可。
6. 按底层 NFT 合约的规则独立配置转出权限；createnft 不做权限处理。
7. 新建 NFT 用 createnft；接入库存用 regstock。初始状态均为下架，确认后 setstatus 上架。

管理员密钥能管理销售参数，但不能代替外部 NFT 发行者授权。生产上线前应核对角色隔离、
RAM/CPU 预算及目标链 ABI，并完成独立安全审查。本仓库不会自动部署生产合约。

## 商品及编号

createnft 自动从 6000000001 开始递增分配 NFT ID，跳过目标合约已占用 ID；每次最多扫描
100 个。超过时失败，由管理员核对后用 advancenft 向前推进。失败创建不消耗编号。
createnft 另接收必填字符串 product_code（1–160 字节），用于业务关联且全局唯一。products.id 仍是数字主键，购买 memo 使用 product_code，其他管理方法的数字 product_id 参数不变。regstock 也必须提供唯一 product_code。
regstock 保留已有 ID。同一资产用 `(nft_contract, nft_id)` 唯一识别，支付使用本地 product_id。
本版 nsymbol 完整字段为 nid，不隐含额外 precision。

- mint=true：商城为发行者，购买时 issue 到商城再 transfer 给付款人。
- mint=false：只转出商城持有的余额，不要求发行权限，不自动铸造。
- quota 是累计销售配额，sold 是累计已售；可售配额为 quota-sold。
- mint 商品在商城维护配额，商城不写入或清理底层 reserved；发行量调整仍检查累计铸造量及剩余配额。
- 库存商品配额不得超过当前真实可用余额；补货不自动增加配额。
- status：0 下架，1 上架，2 永久归档。归档前须释放剩余配额；保留历史记录。

sale_terms 包含 price、starts（UTC 时间）、per_purchase、per_account。
价格和两种限购必须为正，单次上限不超过账户累计上限。修改销售参数前必须下架。
每个账户的成功购买量永久按商品累计；转出、销毁及重新上架都不恢复额度。
普通链下订单不能阻止用户直接购买，也不构成链上预占。

## 转账协议

购买：`mall:v1:<product_code>:<expected_quantity>:<request_id>`。
request_id 为 1–64 位 ASCII 字母、数字、连字符或下划线。request_id 仅用于业务关联，不再去重；重付前请先核对原交易是否成功。
金额必须是单价整数倍，计算数量须与预期一致，不自动找零。最小单位整数计算。

充值空投预算：`mallfund:v1`。
NFT 补货：`mallstock:v1`，必须先登记库存商品（可零配额），批量最多 50 项，逐项校验。
如果商城在部署前已持有 NFT，可直接 regstock 并设置配额。

已登记支付合约的非法 memo 或未知 symbol 拒绝；未登记合约的通知被忽略，不能影响账本。
未知资产可能自行转入，但没有记账或提取支持；前端不应引导此类转账。

## 底层发行与转出规则

商城不读取策略表或发送白名单来判断是否允许登记、上架、补货、提取或购买；实际转账由底层 transfer 检查。上架成功不代表一定能发货，底层拒绝时付款、发行、库存和成交记录整笔回滚。

- 原版 ticket.cvnft：按它的 addwhitelist/setnftwhite 规则配置权限，不需要 setpolicy、setreserve 或 issuance 表。例如由 ticket.cvnft 账户调用 addwhitelist，将商城加入白名单即可允许商城发货。其规则也允许接收方白名单等情形，不等同于严格发送方白名单。
- mall.nft：逐 NFT 发送白名单仍由底层强制执行，setpolicy 用于委托管理。商城不自动设置权限。
- 发行空间：存在 issuance 记录时使用累计 minted；没有该记录时使用 tokenstats.supply。原版 ticket.cvnft 销毁后会恢复发行空间，不能据此保证历史累计发行上限。
- setsupply 仍是扩展功能，需要 issuance 和 setmaxsup；原版 ticket.cvnft 不支持。NFT 白名单直接在底层 NFT 合约管理。setmeta 需满足底层 settokenuri 的授权规则。
- 商城不会调用 setreserve。mall.nft 从历史统一白名单版本升级时，仍需执行 setcreator 清理旧标记。

## 资金与管理记录

funds 按代币合约和完整 symbol 分账，sales 是销售收入、drops 是空投预算。
购买只增加 sales；充值只增加 drops；allocfund 需要 treasurer，将 sales 转为 drops。
airdrop 需要 distributor，仅扣 drops；单批 1–50 条同币种转账，唯一 batch_id 防重复。
发放任一项失败全部回滚。没有销售收入对外提现接口。

管理动作携带非零 op_id 和 1–256 字节 reason，operations 全局防重复并保存操作人、时间和
变更前后序列化快照。后端查询操作号确认成功后不要重发。notifybuy 是仅合约可调用的
结构化成交 action，可结合付款和 NFT 发放交易索引；等待链上最终性后核销订单。

下架状态可用 withdrawnft 取回库存模式未被 quota 占用的 NFT；仍受底层白名单限制。
商品与 purchases 累计记录保留以维持限购，运营需持续预算 RAM。

## 创建与权限配置分离

createnft 只执行模板创建、商城商品登记及商城销售配额，不读取 nftcaps、不检查转出白名单，也不调用 setpolicy/setxferwl。
创建完成后，由发行者和白名单管理员通过 NFT 合约独立配置。setstatus、regstock 及购买仍严格检查发货权限。

setnftcaps 和 setxferwl 已从商城删除，原 nftcaps 数据不再使用。scripts/nft-capabilities.py 仅输出只读接口检查结果，不再生成管理调用参数。
创建只要求兼容的 create 和 tokenstats。商城的创建、配额调整及发行上限调整均不调用 setreserve；配额由商城自身记录。

统一授权：init(admin) 由商城自身调用。商品管理检查 productMgmt，资金划拨检查 mallTreasury，空投检查 mallAirdrop，暂停检查 mallPause。各业务方法新增首参数 submitter，要求该账户签名并通过 cisum.auth::checkrole；超级管理员和合约自身可直接授权。本地 addrole/delrole 已移除，授予、撤销角色使用 cisum.auth 的 grantrole/revokerole。

升级：保留旧 config 二进制布局，但原单账户和数组不再授权。新增 authconfig 保存明确指定的 admin，部署后须调用 init(admin)，不会清空商品或成交数据。完整权限配置示例见 API.md。

升级测试：构建新版前保存上一版 WASM/ABI 为 build/mall-verified/roles.mall.cisum.wasm/.abi，运行 python3 tests/mall.cisum/integration.py --roles-fixture。更早单账户版 fixture 使用 legacy.mall.cisum 文件名和 --upgrade-fixture。

业务编号：products 的字符串 product_id 已更名为 product_code，createnft 参数同步更名。purchases 在尾部追加 product_code（string$），保存业务编号；数字 product_id 保持原意。旧商品字符串值无需迁移；旧购买累计行仍可读取。再次购买时累计行会补齐业务编号。历史空业务编号需要使用 setcode 配置后再购买。

回执表已移除：不再写入 receipts，也不按 request_id 去重。相同请求编号再次付款会按新购买处理，仍受限购和配额检查；request_id 保留格式校验及通知用途。后端应根据交易 ID 和实际成交轨迹确认结果，避免盲目重付。升级不会自动删除历史 receipts 行或返还其 RAM，本次节省的是后续回执及索引写入资源。


按业务编号查询：getproduct(product_code) 在 action 控制台输出 product_id: <数字 id>，不要求管理权限，不写表。使用该 id 作为 products 的 lower_bound/upper_bound 查询完整商品；RPC 只读模拟需要节点支持，普通 push action 仍消耗交易资源。当前链不支持 action return value，因此没有 ABI 返回值。

```sh
fucli push action mall.cisum getproduct '{"product_code":"SKU-001"}' -p alice@active
# 假设输出 product_id: 1
fucli get table mall.cisum mall.cisum products --lower 1 --upper 1
fucli push action mall.cisum setcode '{"submitter":"manager","product_id":1,"product_code":"SKU-001","op_id":1201,"reason":"设置业务编号"}' -p manager@active
```

setcode 需要 productMgmt（或超级管理员/合约自身）授权，商品必须下架，校验编号格式和全表唯一性。历史空编号或重复编号须先修正；重复编号查询会报 ambiguous legacy product_code，不会任意选择商品。编号更改后旧 memo 不再有效，累计限购仍按数字 id 保留，purchases 在下次购买时更新业务编号。

products 追加第三索引 bycode（SHA-256(product_code)，key_type=sha256）。新建或更新商品会维护索引；旧行未自动建立索引，故 getproduct、购买查找和唯一性校验扫描主表，保证升级兼容，成本随商品数量增长。直接 RPC 查第三索引只适用于已建立该索引的行。

购买 memo 为 mall:v1:SKU-001:2:order1，不再把第三段解释为数字 id；纯数字业务编号也按字符串精确匹配。归档商品仍占用编号。createnft、regstock 和 setcode 均拒绝重复编号。


累计购买量查询：getpurchase(buyer, product_code)，buyer 为付款账户，product_code 为当前唯一商品编号。控制台输出 product_id: <数字 id>, quantity: <累计数量>。商品存在但该买家未购买时 quantity 为 0；买家不存在、编号不存在或历史编号重复时拒绝。不要求管理权限、不修改表。

```sh
fucli push action mall.cisum getpurchase '{"buyer":"cloudwudy123","product_code":"SKU-001"}' -p cloudwudy123@active
```

接口先按当前 product_code 找到商品 id，再读取买家 scope 下 purchases，因此兼容尚无业务编号字段的旧累计行；setcode 改名后使用新编号仍能查询原累计量。

## Docker 编译并部署

完整环境准备、私钥与钱包处理、部署后 add-code / init，以及底层白名单和 creator 配置见 [部署说明](DEPLOY.md)。

```sh
bash scripts/deploy-mall.sh --build-only
bash scripts/deploy-mall.sh \
  --rpc https://t.flonscan.io \
  --account nftcreator11 \
  --wallet-url http://host.docker.internal:8900
```

脚本使用 Docker CDT 编译 mall.cisum，再用 Docker fucli 部署本次生成的 WASM/ABI；编译失败不会使用旧文件继续部署。钱包服务须从容器可访问、已解锁并持有相应密钥；不会导入私钥、创建账户或调用 init。钱包实际端口不同请修改 --wallet-url；不要把容器 localhost 当作宿主地址。

默认签名权限为部署账户 @active，可用 --permission 指定。添加 --add-code 会在部署成功后为 active 追加自身代码执行权限；若此步骤失败，前面的合约部署不会撤销。首次初始化或 ABI 参数升级请按 API.md 操作。完整选项见 bash scripts/deploy-mall.sh --help。
