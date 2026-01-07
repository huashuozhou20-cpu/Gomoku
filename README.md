# Gomoku (Win32 + GDI)

A lightweight Windows-only Gomoku game written in C++17 with a native Win32/GDI GUI and a simple heuristic AI. No third-party dependencies.

## Download & Run

- Grab the latest `gomoku.exe` from **GitHub Releases**: https://github.com/<your-org-or-user>/Gomoku/releases
- Double-click `gomoku.exe` to play.

## Fastest Run (Windows)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
.\build\Release\gomoku.exe
```

## Build from Source (Windows)

Requires **Visual Studio C++ Build Tools** and **CMake**.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Run the game:

```powershell
.\build\Release\gomoku.exe
```

Or use the convenience script:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run.ps1
```

## Controls

- **Left click:** place a stone on the nearest intersection.
- **ESC:** return to the menu.
- **R:** restart the current match with the same settings.
- **U:** undo (removes the last AI+human turn when possible).

## Menu & Match Flow

- Choose who moves first (Player or AI).
- Select AI difficulty (Easy/Normal/Hard).
- **Start** begins a match, **Quit** exits.
- After a match ends, use **Play Again** or **Back to Menu**.

## Gameplay Rules

- 15x15 board, first to five in a row wins.
- The first mover plays **black**; the second plays **white**.
