# mall.cisum 部署说明

本文使用测试链 `https://t.flonscan.io`，商城部署账户为 `nftcreator11`，管理员示例为 `manager`。请按实际环境替换账户、代币精度、钱包端口；`mall.cisum` 是代码名称，不要求链上账户同名。

执行顺序：**准备环境及钱包 → 编译部署 → add-code → init(admin) → addbank → 配置底层白名单及 NFT creator → 创建/接入商品 → 配置逐 NFT 权限 → 上架验证**。

## 1. 环境与账户准备

- macOS 安装并启动 Docker Desktop；Linux 安装 Docker Engine。需要 Bash，编译器、CMake、fucli 使用容器内版本，无需在宿主机安装。
- 本地需有 `floncdt:v0.8.1`、`fucli:v0.8.3` 镜像。它们是本项目使用的镜像名，不保证公共仓库可直接拉取；没有时向环境维护者取得镜像或按项目环境构建，并核对标签。
- 可访问目标链 RPC，执行前核对链 ID，避免把测试环境部署到其他链。
- 商城、admin、代币及 NFT 合约账户均已创建；商城账户有足够 RAM、CPU/NET 等链上资源。底层资产合约须已部署、初始化，支付币种须已发行。
- 准备商城部署账户的签名权限、admin 的业务签名权限；外部代币/NFT 权限配置还需要对应合约管理者签名。不同负责人可以分别执行，不必把所有私钥交给部署人员。
- 普通业务账户使用 `cisum.auth` 的角色授权，目标链应准备相应合约和角色。本文用 `init` 指定的超级管理员执行配置，可直接授权；其他账户管理商品须有 `productMgmt`。

在仓库根目录检查：

```bash
docker info
docker image inspect floncdt:v0.8.1 fucli:v0.8.3
bash scripts/deploy-mall.sh --help
```

## 2. 私钥与钱包

部署脚本不接收、不保存私钥，也不自动创建或解锁钱包。交易由钱包服务签名；`--wallet-url` 是钱包 RPC 地址，不是链 RPC。优先使用已有的受控钱包服务。

使用账户 `active` 权限对应的密钥，先核对链上公钥和权限阈值。不要把私钥写进脚本、Git、`.env`、文档、命令参数或日志；不要使用测试代码里的开发私钥。钱包文件和密码分别安全备份，钱包密码不等于账户私钥。多签账户需采用其既定签名流程，单个私钥未必足以部署。

### 可选：在本机启动专用 Docker 钱包

下面在宿主机仅发布回环端口，钱包数据保存在独立 Docker volume。钱包 RPC 没有供公网开放使用的登录保护，禁止发布到公网或共享网络。

```bash
docker volume create mall-deploy-wallet
docker run -d --name mall-deploy-wallet --platform linux/amd64 \
  -p 127.0.0.1:8900:8900 \
  -v mall-deploy-wallet:/wallet \
  --entrypoint fuwal fucli:v0.8.3 \
  --wallet-dir /wallet --http-server-address 0.0.0.0:8900
```

首次创建钱包，屏幕会显示钱包密码，应直接保存到密码管理器，不要将终端输出录入日志：

```bash
docker exec -it mall-deploy-wallet fucli --wallet-url http://127.0.0.1:8900 \
  wallet create --name mall-deploy --to-console
```

交互导入私钥：不传 `--private-key`，按提示输入。根据本次操作需要导入商城或 admin 的签名密钥；外部合约负责人可以在自己的钱包中操作。

```bash
docker exec -it mall-deploy-wallet fucli --wallet-url http://127.0.0.1:8900 \
  wallet import --name mall-deploy
```

钱包重启或自动锁定后，交互输入密码解锁：

```bash
docker exec -it mall-deploy-wallet fucli --wallet-url http://127.0.0.1:8900 \
  wallet unlock --name mall-deploy
```

下文以 macOS Docker Desktop 为例，部署容器通过 `http://host.docker.internal:8900` 访问宿主钱包端口。容器内 `localhost` 指该容器自身。Linux 应提供部署容器可达的受控钱包地址；不能直接照搬 macOS 地址，也不要为解决连通性把钱包暴露到公网。

## 3. 编译并部署

```bash
bash scripts/deploy-mall.sh \
  --rpc https://t.flonscan.io \
  --account nftcreator11 \
  --wallet-url http://host.docker.internal:8900
```

脚本先用 Docker CDT 编译，再部署本次生成的 WASM/ABI。产物保存在 `build/mall.cisum/`。编译失败会停止，不会部署旧产物。只验证编译可使用 `--build-only`；指定签名权限可使用 `--permission nftcreator11@active`。

以下函数方便继续通过同一 Docker CLI 调用，须在当前终端定义；不包含任何私钥：

```bash
mallcli() {
  docker run --rm -i --platform linux/amd64 \
    --entrypoint fucli fucli:v0.8.3 \
    -u https://t.flonscan.io \
    --wallet-url http://host.docker.internal:8900 "$@"
}
mallcli get info
mallcli get account nftcreator11
```

## 4. 部署成功后添加代码执行权限

```bash
mallcli set account permission nftcreator11 active --add-code \
  -p nftcreator11@active
```

`--add-code` 是 CLI 选项，不是商城 action；它为商城账户追加自身代码执行权限，使合约可以内联调用 NFT 创建、发行、转账及权限校验等动作。它不等于底层 NFT 的 creator 权限，也不会添加转账白名单。

也可在第 3 步的部署命令追加 `--add-code`，脚本会在部署成功后执行这一步，二选一即可。若链上权限配置要求上级权限签名，由相应权限负责人执行。此步骤失败不会撤销已经成功的合约部署，修复权限后继续配置即可。

## 5. 初始化 admin

`manager` 必须已存在；此交易由商城账户签名，而不是新 admin 签名。

```bash
mallcli push action nftcreator11 init '{"admin":"manager"}' \
  -p nftcreator11@active
mallcli get table nftcreator11 nftcreator11 authconfig
```

确认 `admin` 为预期账户。`init` 可再次调用以更换超级管理员，不会清空商品、成交或资金；升级时不要无意改掉已有 admin。部署脚本不会自动调用 `init`。

## 6. 登记资产合约 addbank

以下以 `cisum.token` 的 `4,CISUM` 和 `ticket.cvnft` 为例。实际使用 `testtoken111` 的 TOKEN 时，需同时替换 bank 和完整 symbol，例如 `4,TOKEN`。

```bash
mallcli push action nftcreator11 addbank \
  '{"submitter":"manager","bank":"cisum.token","nft":false,"sym":"4,CISUM"}' \
  -p manager@active
mallcli push action nftcreator11 addbank \
  '{"submitter":"manager","bank":"ticket.cvnft","nft":true,"sym":"4,CISUM"}' \
  -p manager@active
```

`submitter` 为操作人；`bank` 为底层合约；`nft` 标识 NFT 或普通代币；`sym` 是完整币种及精度，NFT 分支仍需传合法参数。同一支付币种不要重复登记。`addbank` 由超级管理员或有 `productMgmt` 的账户执行，不要求商城账户自身签名。

**addbank 只修改商城登记，不会自动调用外部白名单、setcreator、setpolicy 或 setreserve。接下来必须由对应合约的权限负责人配置。**

## 7. 底层代币白名单

仓库中的 `cisum.token` 使用 `addwhitelist(account)`，由代币合约自身或其 admin 签名：

```bash
mallcli push action cisum.token addwhitelist '{"account":"nftcreator11"}' \
  -p cisum.token@active
```

这允许商城作为发送方转出代币，例如空投、资金划拨。该版本对普通用户支付还有消费白名单要求：仅将商城加入上述发送白名单，不一定允许普通用户向商城付款。需要接受普通用户消费时，还应配置：

```bash
mallcli push action cisum.token addconsumewl '{"account":"nftcreator11"}' \
  -p cisum.token@active
```

此配置会启用该代币合约的消费统计及相关勋章逻辑，须确认代币自身配置完整。其他代币合约以实际 ABI 和转账规则为准；标准 token 可能没有白名单接口，不能盲目调用这两个方法。

## 8. NFT 白名单与 creator

### 8.1 使用 ticket.cvnft

由 NFT 合约账户签名，把**商城账户**加入转账白名单和 creator 列表：
例子：
```bash
mallcli push action ticket.cvnft addwhitelist '{"acct":"nftcreator11"}' \
  -p ticket.cvnft@active
mallcli push action ticket.cvnft setcreator '{"creator":"nftcreator11","to_add":true}' \
  -p ticket.cvnft@active
```

由商城合约要接受代币（如 CISUM）则需要在代币合约添加对应的消费权限


纯库存销售不调用底层 create，creator 权限不替代库存转入和转出的白名单；如商城还要创建新品，仍须完成 creator 授权。

## 9. 上架前验收与收尾

1. 查询 `authconfig` 确认 admin，查询 `banks` 确认资产登记：`mallcli get table nftcreator11 nftcreator11 banks`。
2. 按 [API.md](API.md) 调用 `createnft` 或 `regstock`；查询 `products` 获取实际商品 ID、NFT ID。新商品默认下架，不会创建后立即开售。
3. 核对 NFT 白名单、creator/DID、库存或发行额度。使用 `mall.nft` 时完成逐 NFT 发送授权，再调用 `setstatus` 上架。
4. 核对开售时间（UTC）、价格、支付币种、单次和每账户限购，用测试账户小额支付验证发货。memo 格式为 `mall:v1:<product_code>:<expected_quantity>:<request_id>`；支付金额须与数量匹配。确认到账 NFT 和买家 scope 下的 `purchases` 数据。支付币留在商城账户。
5. 记录部署交易 ID、WASM/ABI 及配置交易 ID。升级前保留旧产物，核对数据布局兼容性；本脚本不执行数据迁移。

完成签名工作后锁定专用钱包并停止服务（不删除 volume）：

```bash
docker exec mall-deploy-wallet fucli --wallet-url http://127.0.0.1:8900 wallet lock_all
docker stop mall-deploy-wallet
```

后续通过 `docker start mall-deploy-wallet` 启动并交互解锁。使用现有共享钱包时，按其负责人规定收尾，不要直接停止他人的服务。
