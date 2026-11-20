#!/usr/bin/env bash
export PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export ROOT_DIR=${PROJECT_DIR}/

INSTALL_LOCATION="$HOME/flon/cisum.contracts"

set -eo pipefail

function usage() {
  printf "Usage: $0 OPTION...
  -t DIR      Give the directory where Chain Core Lib is installed, and build unit tests.
  -i DIR      Directory to use for installing contracts (Default: ${INSTALL_LOCATION})
  -m TARGET   make target.(Default is empty)
  -d          Build contracts using Docker (default on macOS).
  -y          Non-interactive mode (Uses defaults for each prompt.)
  -h          Print this help menu.
   \\n" "$0" 1>&2
  exit 1
}

BUILD_TESTS=false
USE_DOCKER=false
[[ $(uname -s) == Darwin ]] && USE_DOCKER=true

if [ $# -ne 0 ]; then
  while getopts "t:c:i:m:ydh" opt; do
    case "${opt}" in
    t)
      CHAIN_CORE_INSTALL_DIR=$OPTARG
      BUILD_TESTS=true
    ;;
    i)
      INSTALL_LOCATION=$OPTARG
      ;;
    m)
      MAKE_TARGET=$OPTARG
      ;;
    d)
      USE_DOCKER=true
      ;;
    y)
      NONINTERACTIVE=true
      PROCEED=true
      ;;
    h)
      usage
      ;;
    ?)
      echo "Invalid Option!" 1>&2
      usage
      ;;
    :)
      echo "Invalid Option: -${OPTARG} requires an argument." 1>&2
      usage
      ;;
    *)
      usage
      ;;
    esac
  done
fi

if [[ ${USE_DOCKER} == true && ${BUILD_TESTS} == false ]]; then
  command -v docker >/dev/null || { echo 'Docker is required for this build. Start Docker Desktop and retry.' >&2; exit 1; }
  CDT_IMAGE=${CDT_IMAGE:-fullon/flon-cdt:0.9.0-fixed}
  echo "Building contracts with ${CDT_IMAGE}; output: ${PROJECT_DIR}/build/docker/contracts"
  docker run --rm --platform linux/amd64 --entrypoint bash \
    -e CMAKE_OPTIONS="${CMAKE_OPTIONS:-}" \
    -v "${PROJECT_DIR}:/workspace" -w /workspace "$CDT_IMAGE" -lc '
      set -eo pipefail
      toolchain=$(find /usr/opt -name CDTWasmToolchain.cmake -print -quit)
      [[ -n "$toolchain" ]] || { echo "FLON CDT toolchain not found" >&2; exit 1; }
      cmake -S . -B build/docker -DCMAKE_TOOLCHAIN_FILE="$toolchain" -DBUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX="$1" ${CMAKE_OPTIONS:-}
      if [[ -n "$2" ]]; then
        cmake --build build/docker --parallel 4 --target "$2"
      else
        cmake --build build/docker --parallel 4
      fi
    ' bash "$INSTALL_LOCATION" "${MAKE_TARGET:-}"
  exit 0
fi

# Source helper functions and variables.
TEST_DIR=${PROJECT_DIR}/tests
. ${ROOT_DIR}/scripts/.environment
. ${ROOT_DIR}/scripts/helper.sh

if [[ ${BUILD_TESTS} == true ]]; then
echo "CHAIN_CORE_INSTALL_DIR=${CHAIN_CORE_INSTALL_DIR}"
  if [[ "$CHAIN_CORE_INSTALL_DIR" == "" ]]; then
    echo "Chain Core Lib directory is required when building tests." 1>&2
    usage
  fi
  if [[ ! -d $CHAIN_CORE_INSTALL_DIR ]]; then
    echo "Chain Core Lib directory does not exist: $CHAIN_CORE_INSTALL_DIR" 1>&2
    usage
  fi

  # Include CHAIN_CORE_INSTALL_DIR in CMAKE_PREFIX_PATH
  echo "Using CHAIN_CORE installation at: $CHAIN_CORE_INSTALL_DIR"
  [[ ! -z ${CMAKE_PREFIX_PATH} ]] && CMAKE_PREFIX_PATH=":${CMAKE_PREFIX_PATH}"
  export CMAKE_PREFIX_PATH="${CHAIN_CORE_INSTALL_DIR}${CMAKE_PREFIX_PATH}"
fi

if [[ "${CMAKE_OPTIONS}" != "" ]]; then
  echo "Using CMAKE_OPTIONS=${CMAKE_OPTIONS}"
fi

printf "\t=========== Building flon.contracts ===========\n\n"
RED='\033[0;31m'
NC='\033[0m'
CPU_CORES=$(getconf _NPROCESSORS_ONLN)
mkdir -p build
pushd build &>/dev/null
SDK_OPTIONS=()
if [[ $(uname -s) == Darwin ]]; then
  XCODE_SDK="$(xcode-select -p)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
  [[ ! -d "$XCODE_SDK" ]] || SDK_OPTIONS=(-DCMAKE_OSX_SYSROOT="$XCODE_SDK")
fi
cmake "${SDK_OPTIONS[@]}" -DBUILD_TESTS=${BUILD_TESTS} -DCMAKE_INSTALL_PREFIX="${INSTALL_LOCATION}" -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}" ${CMAKE_OPTIONS} ../
make -j $CPU_CORES ${MAKE_TARGET}
popd &>/dev/null
