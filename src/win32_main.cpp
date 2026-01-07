#include <windows.h>
#include <windowsx.h>

#include <string>

#include "gomoku.h"
#include "gomoku_ai.h"

namespace {

constexpr int kCellSize = 32;
constexpr int kMargin = 40;
constexpr int kInfoHeight = 40;
constexpr int kAiTimerId = 1;
constexpr UINT kAiDelayMs = 120;

struct GameState {
    GomokuGame game;
    int winner = GomokuGame::kEmpty;
    bool ai_pending = false;
};

int BoardPixelSize() {
    return kCellSize * (GomokuGame::kBoardSize - 1);
}

void ResetGame(GameState &state, HWND hwnd) {
    state.game.reset();
    state.game.setCurrentPlayer(GomokuGame::kBlack);
    state.winner = GomokuGame::kEmpty;
    state.ai_pending = false;
    KillTimer(hwnd, kAiTimerId);
    InvalidateRect(hwnd, nullptr, TRUE);
}

void StartAiTimer(GameState &state, HWND hwnd) {
    state.ai_pending = true;
    SetTimer(hwnd, kAiTimerId, kAiDelayMs, nullptr);
}

std::wstring BuildStatusText(const GameState &state) {
    if (state.winner == GomokuGame::kBlack) {
        return L"Winner: Black (You)";
    }
    if (state.winner == GomokuGame::kWhite) {
        return L"Winner: White (AI)";
    }
    if (state.game.isBoardFull()) {
        return L"Draw!";
    }
    if (state.game.currentPlayer() == GomokuGame::kBlack) {
        return L"Turn: Black (You)";
    }
    return L"Turn: White (AI)";
}

void DrawBoard(HDC dc, const RECT &client, const GameState &state) {
    HDC mem_dc = CreateCompatibleDC(dc);
    HBITMAP mem_bitmap = CreateCompatibleBitmap(dc, client.right - client.left, client.bottom - client.top);
    HBITMAP old_bitmap = static_cast<HBITMAP>(SelectObject(mem_dc, mem_bitmap));

    HBRUSH background = CreateSolidBrush(RGB(245, 235, 210));
    FillRect(mem_dc, &client, background);
    DeleteObject(background);

    HPEN grid_pen = CreatePen(PS_SOLID, 1, RGB(60, 60, 60));
    HPEN old_pen = static_cast<HPEN>(SelectObject(mem_dc, grid_pen));

    int board_size = BoardPixelSize();
    for (int i = 0; i < GomokuGame::kBoardSize; ++i) {
        int x = kMargin + i * kCellSize;
        int y = kMargin + i * kCellSize;
        MoveToEx(mem_dc, kMargin, y, nullptr);
        LineTo(mem_dc, kMargin + board_size, y);
        MoveToEx(mem_dc, x, kMargin, nullptr);
        LineTo(mem_dc, x, kMargin + board_size);
    }

    SelectObject(mem_dc, old_pen);
    DeleteObject(grid_pen);

    int stone_radius = kCellSize / 2 - 2;
    for (int y = 0; y < GomokuGame::kBoardSize; ++y) {
        for (int x = 0; x < GomokuGame::kBoardSize; ++x) {
            int cell = state.game.at(x, y);
            if (cell == GomokuGame::kEmpty) {
                continue;
            }
            int cx = kMargin + x * kCellSize;
            int cy = kMargin + y * kCellSize;
            COLORREF color = (cell == GomokuGame::kBlack) ? RGB(30, 30, 30) : RGB(235, 235, 235);
            HBRUSH stone_brush = CreateSolidBrush(color);
            HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(mem_dc, stone_brush));
            Ellipse(mem_dc, cx - stone_radius, cy - stone_radius, cx + stone_radius, cy + stone_radius);
            SelectObject(mem_dc, old_brush);
            DeleteObject(stone_brush);
        }
    }

    std::wstring status = BuildStatusText(state);
    RECT text_rect{
        kMargin,
        kMargin + board_size + 6,
        client.right - kMargin,
        client.bottom - kMargin
    };
    SetBkMode(mem_dc, TRANSPARENT);
    SetTextColor(mem_dc, RGB(20, 20, 20));
    DrawTextW(mem_dc, status.c_str(), static_cast<int>(status.size()), &text_rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    BitBlt(dc, 0, 0, client.right - client.left, client.bottom - client.top, mem_dc, 0, 0, SRCCOPY);

    SelectObject(mem_dc, old_bitmap);
    DeleteObject(mem_bitmap);
    DeleteDC(mem_dc);
}

void HandleUndo(GameState &state, HWND hwnd) {
    if (state.ai_pending) {
        KillTimer(hwnd, kAiTimerId);
        state.ai_pending = false;
    }

    const auto &history = state.game.moveHistory();
    if (history.empty()) {
        return;
    }

    if (history.back().player == GomokuGame::kWhite && history.size() >= 2) {
        state.game.undoLastMove();
        state.game.undoLastMove();
    } else {
        state.game.undoLastMove();
    }

    state.winner = GomokuGame::kEmpty;
    InvalidateRect(hwnd, nullptr, TRUE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    GameState *state = reinterpret_cast<GameState *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (message) {
        case WM_CREATE: {
            auto *created_state = new GameState{};
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(created_state));
            return 0;
        }
        case WM_DESTROY: {
            if (state) {
                delete state;
            }
            PostQuitMessage(0);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            if (!state || state->winner != GomokuGame::kEmpty || state->ai_pending) {
                return 0;
            }
            if (state->game.currentPlayer() != GomokuGame::kBlack) {
                return 0;
            }
            int mouse_x = GET_X_LPARAM(lparam);
            int mouse_y = GET_Y_LPARAM(lparam);
            int col = (mouse_x - kMargin + kCellSize / 2) / kCellSize;
            int row = (mouse_y - kMargin + kCellSize / 2) / kCellSize;
            if (col < 0 || col >= GomokuGame::kBoardSize || row < 0 || row >= GomokuGame::kBoardSize) {
                return 0;
            }
            if (!state->game.placeStone(col, row, GomokuGame::kBlack)) {
                return 0;
            }
            if (state->game.checkWin(col, row, GomokuGame::kBlack)) {
                state->winner = GomokuGame::kBlack;
            } else if (!state->game.isBoardFull()) {
                state->game.setCurrentPlayer(GomokuGame::kWhite);
                StartAiTimer(*state, hwnd);
            }
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        case WM_TIMER: {
            if (!state || wparam != kAiTimerId) {
                return 0;
            }
            KillTimer(hwnd, kAiTimerId);
            state->ai_pending = false;
            if (state->winner != GomokuGame::kEmpty || state->game.isBoardFull()) {
                return 0;
            }
            if (state->game.currentPlayer() != GomokuGame::kWhite) {
                return 0;
            }
            auto move = ComputeAiMove(state->game);
            if (state->game.placeStone(move.first, move.second, GomokuGame::kWhite)) {
                if (state->game.checkWin(move.first, move.second, GomokuGame::kWhite)) {
                    state->winner = GomokuGame::kWhite;
                } else {
                    state->game.setCurrentPlayer(GomokuGame::kBlack);
                }
            }
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        case WM_KEYDOWN: {
            if (!state) {
                return 0;
            }
            if (wparam == 'R') {
                ResetGame(*state, hwnd);
                return 0;
            }
            if (wparam == 'U') {
                HandleUndo(*state, hwnd);
                return 0;
            }
            return 0;
        }
        case WM_PAINT: {
            if (!state) {
                break;
            }
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd, &ps);
            RECT client;
            GetClientRect(hwnd, &client);
            DrawBoard(dc, client, *state);
            EndPaint(hwnd, &ps);
            return 0;
        }
        default:
            return DefWindowProc(hwnd, message, wparam, lparam);
    }
    return 0;
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show_cmd) {
    const wchar_t kClassName[] = L"GomokuWin32";

    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = kClassName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));

    if (!RegisterClassW(&wc)) {
        return 0;
    }

    int client_width = kMargin * 2 + BoardPixelSize();
    int client_height = kMargin * 2 + BoardPixelSize() + kInfoHeight;

    RECT window_rect{0, 0, client_width, client_height};
    AdjustWindowRect(&window_rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

    HWND hwnd = CreateWindowExW(
        0,
        kClassName,
        L"Gomoku - Win32 GDI",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top,
        nullptr,
        nullptr,
        instance,
        nullptr
    );

    if (!hwnd) {
        return 0;
    }

    ShowWindow(hwnd, show_cmd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
