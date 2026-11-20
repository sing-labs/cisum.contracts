#!/usr/bin/env bash
# Compile with Docker CDT, then deploy exactly those artifacts using Docker fucli.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RPC='' ACCOUNT='' WALLET='' PERMISSION=''
CDT_IMAGE='floncdt:v0.8.1' CLI_IMAGE='fucli:v0.8.3'
BUILD_ONLY=false ADD_CODE=false
usage() {
  cat <<'HELP'
编译并部署 mall.cisum（不创建账户、不导入私钥、不调用 init）。

用法：
  bash scripts/deploy-mall.sh --build-only
  bash scripts/deploy-mall.sh --rpc URL --account ACCOUNT --wallet-url URL [选项]

必填（部署时）：
  --rpc URL          节点 RPC，如 https://t.flonscan.io
  --account ACCOUNT  实际部署账户，如 nftcreator11
  --wallet-url URL   Docker 容器可访问的已解锁钱包服务
                    macOS 宿主示例：http://host.docker.internal:8900
选项：
  --permission AUTH  部署签名权限，默认 ACCOUNT@active
  --add-code         部署成功后为 ACCOUNT 的 active 追加代码执行权限
  --build-only       仅编译，不访问节点或钱包
  --cdt-image IMAGE  默认 floncdt:v0.8.1
  --cli-image IMAGE  默认 fucli:v0.8.3
  -h, --help         显示帮助

产物：build/mall.cisum/mall.cisum.wasm 和 mall.cisum.abi
注意：容器中的 localhost 指容器自身。钱包必须已有对应签名密钥并已解锁。
HELP
}
fail(){ printf '%s\n' "$*" >&2; exit 1; }
while [[ $# -gt 0 ]]; do
  case "$1" in
    --rpc|--account|--wallet-url|--permission|--cdt-image|--cli-image)
      [[ $# -ge 2 && -n "$2" && "$2" != --* ]] || fail "参数 $1 缺少值"
      case "$1" in
        --rpc) RPC="$2";; --account) ACCOUNT="$2";; --wallet-url) WALLET="$2";;
        --permission) PERMISSION="$2";; --cdt-image) CDT_IMAGE="$2";; --cli-image) CLI_IMAGE="$2";;
      esac
      shift 2;;
    --build-only) BUILD_ONLY=true; shift;;
    --add-code) ADD_CODE=true; shift;;
    -h|--help) usage; exit 0;;
    *) fail "未知参数：$1";;
  esac
done
if ! $BUILD_ONLY; then
  [[ -n "$RPC" && -n "$ACCOUNT" && -n "$WALLET" ]] || fail '部署必须指定 --rpc、--account、--wallet-url'
  [[ "$ACCOUNT" =~ ^[a-z1-5.]{1,12}$ ]] || fail '部署账户需为 1–12 位合法账户名'
  [[ "$RPC" == http://* || "$RPC" == https://* ]] || fail 'RPC 必须为 HTTP(S) URL'
  [[ "$WALLET" == http://* || "$WALLET" == https://* ]] || fail '钱包必须为 HTTP(S) URL'
  PERMISSION="${PERMISSION:-$ACCOUNT@active}"
fi
command -v docker >/dev/null || fail '未找到 docker'
mkdir -p "$ROOT/build/mall.cisum"
STAGE="$(mktemp -d "$ROOT/build/mall-deploy.XXXXXX")"
trap 'rm -rf -- "$STAGE"' EXIT
printf '%s\n' '使用 Docker CDT 编译 mall.cisum...'
docker run --rm --platform linux/amd64 \
  -v "$ROOT:/workspace" -v "$STAGE:/output" -w /workspace "$CDT_IMAGE" bash -lc '
set -euo pipefail
cmake -S contracts -B /tmp/mall-build \
  -DCMAKE_TOOLCHAIN_FILE=/usr/opt/flon.cdt/0.8.1/lib/cmake/flon.cdt/CDTWasmToolchain.cmake \
  -DSYSTEM_ENABLE_CDT_VERSION_CHECK=OFF
cmake --build /tmp/mall-build --target mall.cisum -j2
cp /tmp/mall-build/mall.cisum/mall.cisum.wasm /tmp/mall-build/mall.cisum/mall.cisum.abi /output/
'
[[ -s "$STAGE/mall.cisum.wasm" && -s "$STAGE/mall.cisum.abi" ]] || fail '未生成有效的 WASM/ABI，停止部署'
cp "$STAGE/mall.cisum.wasm" "$STAGE/mall.cisum.abi" "$ROOT/build/mall.cisum/"
printf '编译成功：%s\n' "$ROOT/build/mall.cisum"
$BUILD_ONLY && exit 0
cli() {
  docker run --rm --platform linux/amd64 -v "$STAGE:/artifacts:ro" \
    --entrypoint fucli "$CLI_IMAGE" -u "$RPC" --wallet-url "$WALLET" "$@"
}
printf '检查部署账户：%s\n' "$ACCOUNT"
cli get account "$ACCOUNT" >/dev/null
printf '部署 mall.cisum 到 %s（权限 %s）...\n' "$ACCOUNT" "$PERMISSION"
cli set contract "$ACCOUNT" /artifacts mall.cisum.wasm mall.cisum.abi -p "$PERMISSION"
printf '%s\n' '合约部署成功。'
if $ADD_CODE; then
  printf '%s\n' '为 active 追加代码执行权限...'
  cli set account permission "$ACCOUNT" active --add-code -p "$PERMISSION"
  printf '%s\n' '代码执行权限设置成功。'
fi
