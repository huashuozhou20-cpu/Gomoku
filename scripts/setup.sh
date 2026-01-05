#!/usr/bin/env bash
set -euo pipefail

if [ ! -d "vcpkg" ]; then
  git clone https://github.com/microsoft/vcpkg.git
fi

./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install raylib

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
