Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

function Write-ErrorAndExit($message) {
  Write-Error $message
  exit 1
}

$cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmakeCommand) {
  Write-ErrorAndExit "CMake not found. Install CMake from https://cmake.org/download/ and re-run."
}

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
  Write-ErrorAndExit "MSVC compiler (cl.exe) not found. Install Visual Studio Build Tools with the C++ workload or use Developer PowerShell for VS."
}

$buildDir = Join-Path $repoRoot "build"

Write-Host "Configuring CMake (Release)..."
& $cmakeCommand.Path -S $repoRoot -B $buildDir -G "Visual Studio 17 2022" -A x64

Write-Host "Building (Release)..."
& $cmakeCommand.Path --build $buildDir --config Release

$exePath = Join-Path $buildDir "Release\gomoku.exe"
if (Test-Path $exePath) {
  & $exePath
  exit $LASTEXITCODE
}

Write-ErrorAndExit "Built executable not found at $exePath."
