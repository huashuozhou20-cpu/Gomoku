Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

function Write-ErrorAndExit($message) {
  Write-Error $message
  exit 1
}

function Resolve-CMakePath {
  if ($env:VSINSTALLDIR) {
    $vsCmakeRoot = Join-Path $env:VSINSTALLDIR "Common7\IDE\CommonExtensions\Microsoft\CMake"
    $vsCmakePath = Join-Path $vsCmakeRoot "bin\cmake.exe"
    if (Test-Path $vsCmakePath) {
      return $vsCmakePath
    }

    if (Test-Path $vsCmakeRoot) {
      $candidate = Get-ChildItem -Path $vsCmakeRoot -Filter "cmake.exe" -File -Recurse -ErrorAction SilentlyContinue |
        Select-Object -First 1
      if ($candidate) {
        return $candidate.FullName
      }
    }
  }

  $cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
  if ($cmakeCommand) {
    return $cmakeCommand.Path
  }

  return $null
}

$cmakePath = Resolve-CMakePath
if (-not $cmakePath) {
  Write-ErrorAndExit "CMake not found. Install CMake from https://cmake.org/download/ and re-run."
}

Write-Host "Using CMake: $cmakePath"

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
  Write-ErrorAndExit "MSVC compiler (cl.exe) not found. Install Visual Studio Build Tools with the C++ workload or use Developer PowerShell for VS."
}

$buildDir = Join-Path $repoRoot "build"
$cachePath = Join-Path $buildDir "CMakeCache.txt"

Write-Host "Configuring CMake..."
$configureOutput = & $cmakePath -S $repoRoot -B $buildDir -G "Visual Studio 17 2022" -A x64 2>&1
$configureOutput | Write-Host

if (-not (Test-Path $cachePath)) {
  Write-ErrorAndExit "CMake cache not found at $cachePath. Configure output above."
}

& $cmakePath --build $buildDir --config Debug

$exePath = Join-Path $buildDir "Debug\\gomoku.exe"
if (Test-Path $exePath) {
  & $exePath
  exit $LASTEXITCODE
}

Write-ErrorAndExit "Built executable not found at $exePath."
