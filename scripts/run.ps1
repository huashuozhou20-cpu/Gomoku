$ErrorActionPreference = "Stop"

$debugPath = Join-Path "build" "Debug\gomoku.exe"
$releasePath = Join-Path "build" "gomoku.exe"

if (Test-Path $debugPath) {
  & $debugPath
  exit $LASTEXITCODE
}

if (Test-Path $releasePath) {
  & $releasePath
  exit $LASTEXITCODE
}

Write-Error "build\\Debug\\gomoku.exe or build\\gomoku.exe not found. Run the Build task first."
