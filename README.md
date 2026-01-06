# Gomoku (Five-in-a-Row)

A cross-platform Gomoku game built in C++17. Play as the black stones against a fast heuristic AI in a 15x15 grid. The UI highlights the last move, shows whose turn it is, and lets you restart at any time.

## Controls

- **Left click:** place a stone on an empty cell.
- **R:** restart the game.
- **Restart button (raylib build):** click the on-screen button to reset.

## Download & Run (Recommended)

1. Download the latest release for your OS from **GitHub Releases**: https://github.com/<your-org-or-user>/Gomoku/releases
2. Unzip the archive.
3. Run the `gomoku` binary (`gomoku.exe` on Windows).

## Build from Source (Developers)

### Windows (PowerShell)

```powershell
powershell -ExecutionPolicy Bypass -File .\\scripts\\run.ps1
```

### macOS/Linux

```bash
./scripts/run.sh
```

The run scripts will clone/bootstraps **vcpkg**, configure CMake presets, build, and run the game.

## VS Code Quick Start

### Required extensions

- **C/C++** (ms-vscode.cpptools)
- **CMake Tools** (ms-vscode.cmake-tools)

### Configure, build, run

1. Open the repo in VS Code.
2. Run **Run** (this uses the one-click script).
3. Or run **Configure** then **Build** if you want separate steps.

### Debug

- Press **F5** to debug using the same build output as the Run task.

## Troubleshooting

- **Windows:** Install Visual Studio Build Tools with the C++ workload (or use Developer PowerShell for VS).
- **macOS:** Run `xcode-select --install`.
- **Linux:** `sudo apt-get install build-essential cmake` (or your distro equivalent).
- **PowerShell execution policy:** Use `-ExecutionPolicy Bypass` or set `Set-ExecutionPolicy -Scope CurrentUser RemoteSigned`.
- **vcpkg install failures:** Check your network/proxy settings and re-run the script.

## Gameplay Rules

- Standard Gomoku rules: five in a row (horizontal, vertical, or diagonal) wins.
- The game checks for a win immediately after each move.

## Dependencies

- **raylib** via vcpkg (manifest mode).
- CMake 3.23+ and a C++17 compiler.

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
