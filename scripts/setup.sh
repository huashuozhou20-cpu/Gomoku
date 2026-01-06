#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

if [ ! -d "vcpkg" ]; then
  git clone https://github.com/microsoft/vcpkg.git
fi

./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install raylib

preset="linux-release"
if [ "$(uname -s)" = "Darwin" ]; then
  preset="macos-release"
fi

cmake --preset "$preset"
