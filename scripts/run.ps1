Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

function Write-ErrorAndExit($message) {
  Write-Error $message
  exit 1
}

if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
  Write-ErrorAndExit "git not found. Install Git from https://git-scm.com/downloads and re-run."
}

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
  Write-ErrorAndExit "CMake not found. Install CMake from https://cmake.org/download/ and re-run."
}

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
  Write-ErrorAndExit "MSVC compiler (cl.exe) not found. Install Visual Studio Build Tools with the C++ workload or use Developer PowerShell for VS."
}

if (-not (Test-Path "vcpkg")) {
  Write-Host "Cloning vcpkg..."
  git clone https://github.com/microsoft/vcpkg.git
}

if (-not (Test-Path ".\vcpkg\vcpkg.exe")) {
  Write-Host "Bootstrapping vcpkg..."
  .\vcpkg\bootstrap-vcpkg.bat
}

$toolchainPath = Resolve-Path ".\vcpkg\scripts\buildsystems\vcpkg.cmake"
$env:VCPKG_ROOT = (Resolve-Path ".\vcpkg").Path

$preset = "windows-msvc-debug"
$buildDir = Join-Path "build" $preset

Write-Host "Configuring with preset: $preset"
cmake --preset $preset
cmake --build --preset $preset

$exePath = Join-Path $buildDir "gomoku.exe"
if (Test-Path $exePath) {
  & $exePath
  exit $LASTEXITCODE
}

Write-ErrorAndExit "Built executable not found at $exePath."
