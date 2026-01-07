$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

if (-not (Test-Path "vcpkg")) {
  git clone https://github.com/microsoft/vcpkg.git
}

.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install raylib

cmake --preset windows-msvc-debug
