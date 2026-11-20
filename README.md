# cisum.contracts
cisum verse web3 contracts

## Build

Run `bash ./build.sh` from the repository directory. On macOS this uses Docker and
`fullon/flon-cdt:0.9.0-fixed`, avoiding host Xcode/SDK and native CDT dependencies.
Start Docker Desktop first. WASM/ABI files are written to
`build/docker/contracts/<contract>/`; use these paths when deploying.
The existing `build/contracts/` directory is not updated by Docker builds.

On Linux, use `bash ./build.sh -d` for the same Docker build, or run without
`-d` to use an installed native FLON CDT. Set `CDT_IMAGE` to override the Docker
image. `-t DIR` selects the native unit-test build and requires native CDT and
Chain Core libraries. Docker builds do not run chain integration tests.
