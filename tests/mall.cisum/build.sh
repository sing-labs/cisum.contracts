#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
docker run --rm --platform linux/amd64 -v "$PWD:/workspace" -w /workspace floncdt:v0.8.1 bash -lc '
set -euo pipefail
cmake -S contracts -B /tmp/mall-build -DCMAKE_TOOLCHAIN_FILE=/usr/opt/flon.cdt/0.8.1/lib/cmake/flon.cdt/CDTWasmToolchain.cmake -DSYSTEM_ENABLE_CDT_VERSION_CHECK=OFF
cmake --build /tmp/mall-build --target mall.cisum mall.nft ticket.cvnft cisum.token -j2
mkdir -p build/mall-verified
for contract in mall.cisum mall.nft ticket.cvnft cisum.token; do
  cp /tmp/mall-build/$contract/$contract.wasm /tmp/mall-build/$contract/$contract.abi build/mall-verified/
done
'
