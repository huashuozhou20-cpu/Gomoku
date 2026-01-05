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
    sudo apt-get install -y cmake build-essential git pkg-config
  else
    print_error "apt-get not found. Please install CMake, a C++ compiler, Git, and pkg-config manually."
    exit 1
  fi
}

install_prereqs_macos() {
  if ensure_command brew; then
    echo "Installing CMake via Homebrew..."
    brew install cmake
  else
    print_error "Homebrew not found. Install Homebrew (https://brew.sh/) and then install CMake."
    exit 1
  fi
}

if ! ensure_command git; then
  print_error "git not found. Please install Git and re-run the script."
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
      print_error "Unsupported OS for automatic CMake install. Please install CMake manually."
      exit 1
      ;;
  esac
fi

if ! ensure_command g++ && ! ensure_command clang++; then
  print_error "No C++ compiler found (g++ or clang++). Please install a compiler and re-run."
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

dependency="raylib"
if [ -f "vcpkg.json" ]; then
  if grep -q '"sdl2"' vcpkg.json; then
    dependency="sdl2"
  elif grep -q '"raylib"' vcpkg.json; then
    dependency="raylib"
  fi
else
  if grep -q "find_package(SDL2" CMakeLists.txt; then
    dependency="sdl2"
  fi
fi

echo "Installing dependency: ${dependency}"
./vcpkg/vcpkg install "$dependency"

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build -j

if [ -x "./build/gomoku" ]; then
  exec ./build/gomoku
fi

exe_path="$(find build -maxdepth 3 -type f -perm -111 -name 'gomoku' 2>/dev/null | head -n 1)"
if [ -n "$exe_path" ]; then
  exec "$exe_path"
fi

print_error "Built executable not found. Check the build output in ./build."
exit 1
