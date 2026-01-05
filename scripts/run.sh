#!/usr/bin/env bash
set -euo pipefail

if [ -x "./build/gomoku" ]; then
  exec ./build/gomoku
fi

echo "Error: build/gomoku not found. Run the Build task first." >&2
exit 1
