$ErrorActionPreference = "Stop"

if (-not (Test-Path "vcpkg")) {
  git clone https://github.com/microsoft/vcpkg.git
}

.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install raylib

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
