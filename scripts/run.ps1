Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

function Write-ErrorAndExit($message) {
  Write-Error $message
  exit 1
}

function Get-VsCmakePath {
  $candidateRoots = @()
  if ($env:VSINSTALLDIR) {
    $candidateRoots += $env:VSINSTALLDIR
  } else {
    $candidateRoots += "C:\Program Files\Microsoft Visual Studio"
    $candidateRoots += "C:\Program Files (x86)\Microsoft Visual Studio"
  }

  foreach ($root in $candidateRoots) {
    $vsCmakePath = Join-Path $root "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    if (Test-Path $vsCmakePath) {
      return $vsCmakePath
    }

    if (Test-Path $root) {
      $found = Get-ChildItem -Path $root -Filter cmake.exe -File -Recurse -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -match "CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake\.exe$" } |
        Select-Object -First 1
      if ($found) {
        return $found.FullName
      }
    }
  }

  return $null
}

$cmakePath = Get-VsCmakePath
if (-not $cmakePath) {
  Write-ErrorAndExit "Visual Studio bundled CMake not found. Ensure Visual Studio is installed and VSINSTALLDIR is set."
}

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
  Write-ErrorAndExit "MSVC compiler (cl.exe) not found. Install Visual Studio Build Tools with the C++ workload or use Developer PowerShell for VS."
}

$buildDir = Join-Path $repoRoot "build"

Write-Host "Configuring CMake (Release)..."
& $cmakePath -S $repoRoot -B $buildDir -G "Visual Studio 17 2022" -A x64

Write-Host "Building (Release)..."
& $cmakePath --build $buildDir --config Release

$exePath = Join-Path $buildDir "Release\gomoku.exe"
if (Test-Path $exePath) {
  & $exePath
  exit $LASTEXITCODE
}

Write-ErrorAndExit "Built executable not found at $exePath."
