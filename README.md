# Gomoku (Five-in-a-Row)

A cross-platform Gomoku game built in C++17. Play as the black stones against a fast heuristic AI in a 15x15 grid. The UI highlights the last move, shows whose turn it is, and lets you restart at any time.

## Controls

- **Left click:** place a stone on an empty cell.
- **R:** restart the game.
- **Restart button (raylib build):** click the on-screen button to reset.

## How to Build

This project prefers **raylib** (via vcpkg when available) and will fall back to **SDL2** if raylib is not found in your environment. If neither library is available, a lightweight **X11 fallback** is used for Linux-only builds.

### Option A: vcpkg + raylib (recommended)

```bash
# Install dependencies
vcpkg install raylib

# Configure & build
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

### Option B: vcpkg + SDL2 (fallback)

```bash
# Install dependencies
vcpkg install sdl2

# Configure & build with SDL2 explicitly
cmake -S . -B build -DUSE_SDL2=ON -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

### Option C: Linux X11 fallback (no external packages)

```bash
cmake -S . -B build
cmake --build build
```

> The X11 fallback is intended for restricted environments where vcpkg downloads are blocked. It still provides a full graphical window and UI but is Linux-specific.

### Run

```bash
./build/gomoku
```

## VS Code Quick Start

### Required extensions

- **C/C++** (ms-vscode.cpptools)
- **CMake Tools** (ms-vscode.cmake-tools)

### Configure, build, run

1. Open the repo in VS Code.
2. Run the **Setup** task (clones and bootstraps vcpkg, installs `raylib`, and configures CMake).
3. Run **Build**.
4. Run **Run** to launch the game.

### Debug

- Press **F5** to debug using the same build output as the Run task.

### Common issues

- **Missing compiler:** Install a C++ compiler (MSVC, clang, or GCC).
- **CMake not installed:** Install CMake and ensure it is on your PATH.
- **vcpkg bootstrap issues:** Ensure Git is installed and you have access to GitHub, then re-run **Setup**.
- **Windows build output path differences:** Some generators output to `build/Debug/gomoku.exe` while others use `build/gomoku.exe`. Use the matching debug configuration in `launch.json`.
- **PowerShell execution policy:** If scripts are blocked, run PowerShell as admin and use `Set-ExecutionPolicy -Scope CurrentUser RemoteSigned`, or run tasks with `-ExecutionPolicy Bypass`.

## Gameplay Rules

- Standard Gomoku rules: five in a row (horizontal, vertical, or diagonal) wins.
- The game checks for a win immediately after each move.

## Dependencies

- **raylib** (preferred) for graphics and input.
- **SDL2** is used automatically if raylib is not available (or when `USE_SDL2=ON`).
- **X11** fallback is used when neither raylib nor SDL2 is available.
- CMake 3.16+ and a C++17 compiler.

## Troubleshooting

- **raylib not found:** ensure vcpkg is installed and the toolchain file is passed to CMake.
- **SDL2 linker errors:** confirm you installed `sdl2` via vcpkg and are using `-DUSE_SDL2=ON`.
- **X11 build errors (Linux):** install X11 development headers (e.g., `libx11-dev`).
- **macOS framework errors:** make sure you have Xcode command-line tools installed (`xcode-select --install`).
- **Black window on Linux:** ensure your system has OpenGL development packages installed (e.g., `mesa-common-dev` / `libgl1-mesa-dev`).

## Project Structure

```
.
├── CMakeLists.txt
├── README.md
├── src/
│   ├── game.cpp
│   ├── game.h
│   ├── main_raylib.cpp
│   ├── main_sdl2.cpp
│   └── main_x11.cpp
```

## How to Play

1. Launch the game.
2. Click to place your stone.
3. The AI replies immediately.
4. Get five in a row to win!
