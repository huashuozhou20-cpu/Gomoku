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

$dependency = "raylib"
if (Test-Path "vcpkg.json") {
  $manifest = Get-Content "vcpkg.json" -Raw
  if ($manifest -match '"sdl2"') {
    $dependency = "sdl2"
  } elseif ($manifest -match '"raylib"') {
    $dependency = "raylib"
  }
} else {
  if (Select-String -Path "CMakeLists.txt" -Pattern "find_package\(SDL2" -Quiet) {
    $dependency = "sdl2"
  }
}

Write-Host "Installing dependency: $dependency"
.\vcpkg\vcpkg install $dependency

if (Test-Path "build" -PathType Leaf) {
  Remove-Item "build" -Force
}

if (-not (Test-Path "build" -PathType Container)) {
  New-Item -ItemType Directory -Path "build" | Out-Null
}

cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=.\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Debug

$debugExe = Join-Path "build" "Debug\gomoku.exe"
$releaseExe = Join-Path "build" "Release\gomoku.exe"
$singleConfigExe = Join-Path "build" "gomoku.exe"

if (Test-Path $debugExe) {
  & $debugExe
  exit $LASTEXITCODE
}

if (Test-Path $releaseExe) {
  & $releaseExe
  exit $LASTEXITCODE
}

if (Test-Path $singleConfigExe) {
  & $singleConfigExe
  exit $LASTEXITCODE
}

$foundExe = Get-ChildItem -Path "build" -Filter "gomoku*.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
if ($foundExe) {
  & $foundExe.FullName
  exit $LASTEXITCODE
}

Write-ErrorAndExit "Built executable not found. Check the build output in .\build."
