#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

print_error() {
  echo "Error: $1" >&2
}

ensure_command() {
  local cmd="$1"
  if ! command -v "$cmd" >/dev/null 2>&1; then
    return 1
  fi
  return 0
}

install_prereqs_linux() {
  if ensure_command apt-get; then
    echo "Installing prerequisites via apt-get..."
    sudo apt-get update
    sudo apt-get install -y build-essential cmake git pkg-config
  else
    print_error "apt-get not found. Install build-essential, cmake, git, and pkg-config manually."
    exit 1
  fi
}

install_prereqs_macos() {
  if ensure_command brew; then
    echo "Installing prerequisites via Homebrew..."
    brew install cmake
  else
    print_error "Homebrew not found. Install Homebrew (https://brew.sh/) and then install CMake."
    exit 1
  fi
}

if ! ensure_command git; then
  print_error "git not found. Install Git and re-run."
  exit 1
fi

if ! ensure_command cmake; then
  case "$(uname -s)" in
    Darwin)
      install_prereqs_macos
      ;;
    Linux)
      install_prereqs_linux
      ;;
    *)
      print_error "Unsupported OS for automatic setup. Install CMake manually."
      exit 1
      ;;
  esac
fi

if ! ensure_command g++ && ! ensure_command clang++; then
  print_error "No C++ compiler found (g++ or clang++). Install one and re-run."
  exit 1
fi

if [ ! -d "vcpkg" ]; then
  echo "Cloning vcpkg..."
  git clone https://github.com/microsoft/vcpkg.git
fi

if [ ! -x "./vcpkg/vcpkg" ]; then
  echo "Bootstrapping vcpkg..."
  ./vcpkg/bootstrap-vcpkg.sh
fi

preset="linux-release"
if [ "$(uname -s)" = "Darwin" ]; then
  preset="macos-release"
fi

echo "Configuring with preset: ${preset}"
cmake --preset "${preset}"
cmake --build --preset "${preset}"

exe_path="./build/${preset}/gomoku"
if [ -x "$exe_path" ]; then
  exec "$exe_path"
fi

print_error "Built executable not found at ${exe_path}."
exit 1
