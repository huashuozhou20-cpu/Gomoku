#include <windows.h>
#include <windowsx.h>

#include <cmath>
#include <string>
#include <vector>

#include "gomoku.h"
#include "gomoku_ai.h"

namespace {

constexpr int kCellSize = 32;
constexpr int kMargin = 40;
constexpr int kAiTimerId = 1;
constexpr int kBlinkTimerId = 2;
constexpr int kPulseTimerId = 3;
constexpr UINT kBlinkIntervalMs = 300;
constexpr UINT kPulseIntervalMs = 30;
constexpr int kPulseFrames = 7;
constexpr int kWindowWidth = 700;
constexpr int kWindowHeight = 820;

enum class Scene {
    Menu,
    Playing,
    GameOver
};

struct GameState {
    GomokuGame game;
    Scene scene = Scene::Menu;
    AiDifficulty difficulty = AiDifficulty::Normal;
    bool player_first = true;
    int human_player = GomokuGame::kBlack;
    int ai_player = GomokuGame::kWhite;
    int winner = GomokuGame::kEmpty;
    std::optional<WinLine> win_line;
    std::vector<POINT> win_cells;
    bool win_blink_on = true;
    int pulse_frame = 0;
    int pulse_total = 0;
    bool ai_pending = false;
};

static void StartAiTimer(HWND hwnd);
static void DoAiMove(GameState &state, HWND hwnd);

int ScaleByDpi(UINT dpi, int value) {
    return MulDiv(value, static_cast<int>(dpi), 96);
}

int ScaleByDpi(HWND hwnd, int value) {
    UINT dpi = GetDpiForWindow(hwnd);
    return ScaleByDpi(dpi, value);
}

int BoardPixelSize(int cell_size) {
    return cell_size * (GomokuGame::kBoardSize - 1);
}

RECT MakeCenteredRect(const RECT &client, int center_y, int width, int height) {
    int center_x = (client.left + client.right) / 2;
    RECT rect{
        center_x - width / 2,
        center_y - height / 2,
        center_x + width / 2,
        center_y + height / 2
    };
    return rect;
}

struct MenuLayout {
    RECT title{};
    RECT subtitle{};
    RECT first_card{};
    RECT difficulty_card{};
    RECT action_card{};
    RECT player_first{};
    RECT ai_first{};
    RECT easy{};
    RECT normal{};
    RECT hard{};
    RECT start{};
    RECT quit{};
};

struct GameOverLayout {
    RECT play_again{};
    RECT back_to_menu{};
};

MenuLayout BuildMenuLayout(HWND hwnd, const RECT &client) {
    MenuLayout layout{};
    int side_margin = ScaleByDpi(hwnd, 24);
    int card_padding = ScaleByDpi(hwnd, 16);
    int card_gap = ScaleByDpi(hwnd, 18);
    int label_height = ScaleByDpi(hwnd, 18);
    int button_height = ScaleByDpi(hwnd, 40);
    int action_height = ScaleByDpi(hwnd, 52);
    int button_gap = ScaleByDpi(hwnd, 12);
    int content_width = client.right - client.left - side_margin * 2;

    int y = side_margin;
    layout.title = MakeCenteredRect(client, y + ScaleByDpi(hwnd, 28), ScaleByDpi(hwnd, 420), ScaleByDpi(hwnd, 60));
    y += ScaleByDpi(hwnd, 64);
    layout.subtitle = MakeCenteredRect(client, y, ScaleByDpi(hwnd, 420), ScaleByDpi(hwnd, 26));
    y += ScaleByDpi(hwnd, 40);

    int card_height = card_padding * 2 + label_height + button_height;
    layout.first_card = RECT{side_margin, y, client.right - side_margin, y + card_height};
    int row_y = layout.first_card.top + card_padding + label_height;
    int button_width_two = (content_width - button_gap) / 2;
    layout.player_first = RECT{side_margin + card_padding, row_y,
                               side_margin + card_padding + button_width_two, row_y + button_height};
    layout.ai_first = RECT{layout.player_first.right + button_gap, row_y,
                           layout.player_first.right + button_gap + button_width_two, row_y + button_height};

    y = layout.first_card.bottom + card_gap;
    layout.difficulty_card = RECT{side_margin, y, client.right - side_margin, y + card_height};
    row_y = layout.difficulty_card.top + card_padding + label_height;
    int button_width_three = (content_width - button_gap * 2) / 3;
    layout.easy = RECT{side_margin + card_padding, row_y,
                       side_margin + card_padding + button_width_three, row_y + button_height};
    layout.normal = RECT{layout.easy.right + button_gap, row_y,
                         layout.easy.right + button_gap + button_width_three, row_y + button_height};
    layout.hard = RECT{layout.normal.right + button_gap, row_y,
                       layout.normal.right + button_gap + button_width_three, row_y + button_height};

    y = layout.difficulty_card.bottom + card_gap;
    int action_height_total = card_padding * 2 + label_height + action_height * 2 + button_gap;
    layout.action_card = RECT{side_margin, y, client.right - side_margin, y + action_height_total};
    int action_y = layout.action_card.top + card_padding + label_height;
    layout.start = RECT{side_margin + card_padding, action_y,
                        side_margin + card_padding + content_width, action_y + action_height};
    layout.quit = RECT{layout.start.left, layout.start.bottom + button_gap,
                       layout.start.right, layout.start.bottom + button_gap + action_height};

    return layout;
}

GameOverLayout BuildGameOverLayout(HWND hwnd, const RECT &client) {
    GameOverLayout layout{};
    int center_y = (client.top + client.bottom) / 2 + ScaleByDpi(hwnd, 90);
    int button_width = ScaleByDpi(hwnd, 240);
    int button_height = ScaleByDpi(hwnd, 44);
    int button_gap = ScaleByDpi(hwnd, 14);
    layout.play_again = MakeCenteredRect(client, center_y, button_width, button_height);
    layout.back_to_menu = MakeCenteredRect(client, center_y + button_height + button_gap, button_width, button_height);
    return layout;
}

void ConfigurePlayers(GameState &state) {
    if (state.player_first) {
        state.human_player = GomokuGame::kBlack;
        state.ai_player = GomokuGame::kWhite;
    } else {
        state.human_player = GomokuGame::kWhite;
        state.ai_player = GomokuGame::kBlack;
    }
}

RECT MakeCenteredRect(const RECT &client, int center_y, int width, int height) {
    int center_x = (client.left + client.right) / 2;
    RECT rect{
        center_x - width / 2,
        center_y - height / 2,
        center_x + width / 2,
        center_y + height / 2
    };
    return rect;
}

struct MenuLayout {
    RECT player_first{};
    RECT ai_first{};
    RECT easy{};
    RECT normal{};
    RECT hard{};
    RECT start{};
    RECT quit{};
};

struct GameOverLayout {
    RECT play_again{};
    RECT back_to_menu{};
};

MenuLayout BuildMenuLayout(const RECT &client) {
    MenuLayout layout{};
    int center_x = (client.left + client.right) / 2;
    int y = kMargin + 40;
    layout.player_first = RECT{center_x - kOptionWidth - 10, y, center_x - 10, y + kOptionHeight};
    layout.ai_first = RECT{center_x + 10, y, center_x + kOptionWidth + 10, y + kOptionHeight};

    y += kOptionHeight + 40;
    layout.easy = RECT{center_x - kOptionWidth - 10, y, center_x - 10, y + kOptionHeight};
    layout.normal = RECT{center_x + 10, y, center_x + kOptionWidth + 10, y + kOptionHeight};
    layout.hard = RECT{center_x - kOptionWidth / 2, y + kOptionHeight + 10,
                       center_x + kOptionWidth / 2, y + kOptionHeight + 10 + kOptionHeight};

    y += kOptionHeight * 2 + 60;
    layout.start = MakeCenteredRect(client, y, kButtonWidth, kButtonHeight);
    layout.quit = MakeCenteredRect(client, y + kButtonHeight + 20, kButtonWidth, kButtonHeight);

    return layout;
}

GameOverLayout BuildGameOverLayout(const RECT &client) {
    GameOverLayout layout{};
    int center_y = (client.top + client.bottom) / 2 + 40;
    layout.play_again = MakeCenteredRect(client, center_y, kButtonWidth, kButtonHeight);
    layout.back_to_menu = MakeCenteredRect(client, center_y + kButtonHeight + 16, kButtonWidth, kButtonHeight);
    return layout;
}

void ConfigurePlayers(GameState &state) {
    if (state.player_first) {
        state.human_player = GomokuGame::kBlack;
        state.ai_player = GomokuGame::kWhite;
    } else {
        state.human_player = GomokuGame::kWhite;
        state.ai_player = GomokuGame::kBlack;
    }
}

void ResetGame(GameState &state, HWND hwnd) {
    state.game.reset();
    ConfigurePlayers(state);
    state.game.setCurrentPlayer(state.player_first ? state.human_player : state.ai_player);
    state.winner = GomokuGame::kEmpty;
    state.win_line.reset();
    state.win_cells.clear();
    state.win_blink_on = true;
    state.pulse_frame = 0;
    state.pulse_total = 0;
    state.ai_pending = false;
    KillTimer(hwnd, kAiTimerId);
    KillTimer(hwnd, kBlinkTimerId);
    KillTimer(hwnd, kPulseTimerId);
    InvalidateRect(hwnd, nullptr, TRUE);
}

void StartMatch(GameState &state, HWND hwnd) {
    ResetGame(state, hwnd);
    state.scene = Scene::Playing;
    if (state.game.currentPlayer() == state.ai_player) {
        state.ai_pending = true;
        StartAiTimer(hwnd);
    }
}

static void StartAiTimer(HWND hwnd) {
    constexpr UINT_PTR kAiTimerId = 1;
    constexpr UINT kDelayMs = 120;
    KillTimer(hwnd, kAiTimerId);
    SetTimer(hwnd, kAiTimerId, kDelayMs, nullptr);
}

std::wstring BuildStatusText(const GameState &state) {
    if (state.winner == state.human_player) {
        return L"Winner: You";
    }
    if (state.winner == state.ai_player) {
        return L"Winner: AI";
    }
    if (state.game.isBoardFull()) {
        return L"Draw!";
    }
    if (state.game.currentPlayer() == state.human_player) {
        return L"Turn: You";
    }
    return L"Turn: AI";
}

void DrawButton(HDC dc, const RECT &rect, const std::wstring &label, bool selected) {
    HBRUSH brush = CreateSolidBrush(selected ? RGB(180, 210, 240) : RGB(230, 230, 230));
    FillRect(dc, &rect, brush);
    DeleteObject(brush);

    FrameRect(dc, &rect, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(20, 20, 20));
    DrawTextW(dc, label.c_str(), static_cast<int>(label.size()), const_cast<RECT *>(&rect),
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void DrawBoardBase(HDC dc, const RECT &client, const GameState &state) {
    HDC mem_dc = CreateCompatibleDC(dc);
    HBITMAP mem_bitmap = CreateCompatibleBitmap(dc, client.right - client.left, client.bottom - client.top);
    HBITMAP old_bitmap = static_cast<HBITMAP>(SelectObject(mem_dc, mem_bitmap));

    HBRUSH background = CreateSolidBrush(RGB(245, 235, 210));
    FillRect(mem_dc, &client, background);
    DeleteObject(background);

    HPEN grid_pen = CreatePen(PS_SOLID, 1, RGB(60, 60, 60));
    HPEN old_pen = static_cast<HPEN>(SelectObject(mem_dc, grid_pen));

    int board_size = BoardPixelSize(kCellSize);
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
    auto last_move = state.game.lastMove();
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
            int radius = stone_radius;
            if (last_move && last_move->x == x && last_move->y == y && state.pulse_total > 0
                && state.pulse_frame < state.pulse_total) {
                float t = static_cast<float>(state.pulse_frame) / static_cast<float>(state.pulse_total);
                float scale = 1.0f + 0.15f * std::sin(t * 3.1415926f);
                radius = static_cast<int>(stone_radius * scale);
            }
            Ellipse(mem_dc, cx - radius, cy - radius, cx + radius, cy + radius);
            SelectObject(mem_dc, old_brush);
            DeleteObject(stone_brush);
        }
    }

    if (!state.win_cells.empty() && state.win_blink_on) {
        POINT start = state.win_cells.front();
        POINT end = state.win_cells.back();
        int start_cx = kMargin + start.x * kCellSize;
        int start_cy = kMargin + start.y * kCellSize;
        int end_cx = kMargin + end.x * kCellSize;
        int end_cy = kMargin + end.y * kCellSize;

        HPEN win_pen = CreatePen(PS_SOLID, 6, RGB(200, 60, 60));
        HPEN old_win_pen = static_cast<HPEN>(SelectObject(mem_dc, win_pen));
        MoveToEx(mem_dc, start_cx, start_cy, nullptr);
        LineTo(mem_dc, end_cx, end_cy);
        SelectObject(mem_dc, old_win_pen);
        DeleteObject(win_pen);
    }

    if (state.scene == Scene::Playing) {
        std::wstring status = BuildStatusText(state);
        RECT text_rect{
            kMargin,
            kMargin + board_size + 6,
            client.right - kMargin,
            client.bottom - kMargin
        };
        SetBkMode(mem_dc, TRANSPARENT);
        SetTextColor(mem_dc, RGB(20, 20, 20));
        DrawTextW(mem_dc, status.c_str(), static_cast<int>(status.size()), &text_rect,
                  DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    }

    BitBlt(dc, 0, 0, client.right - client.left, client.bottom - client.top, mem_dc, 0, 0, SRCCOPY);

    SelectObject(mem_dc, old_bitmap);
    DeleteObject(mem_bitmap);
    DeleteDC(mem_dc);
}

void DrawMenu(HDC dc, HWND hwnd, const RECT &client, const GameState &state) {
    HBRUSH background = CreateSolidBrush(RGB(248, 245, 238));
    FillRect(dc, &client, background);
    DeleteObject(background);

    int title_size = ScaleByDpi(hwnd, 42);
    int subtitle_size = ScaleByDpi(hwnd, 18);

    HFONT title_font = CreateFontW(title_size, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                   CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HFONT subtitle_font = CreateFontW(subtitle_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(30, 30, 30));

    MenuLayout layout = BuildMenuLayout(hwnd, client);

    HFONT old_font = static_cast<HFONT>(SelectObject(dc, title_font));
    DrawTextW(dc, L"Gomoku", -1, &layout.title, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, subtitle_font);
    DrawTextW(dc, L"Win32 + GDI Edition", -1, &layout.subtitle, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, old_font);
    DeleteObject(title_font);
    DeleteObject(subtitle_font);

    HBRUSH card_brush = CreateSolidBrush(RGB(240, 240, 236));
    HBRUSH border_brush = static_cast<HBRUSH>(GetStockObject(GRAY_BRUSH));
    FillRect(dc, &layout.first_card, card_brush);
    FrameRect(dc, &layout.first_card, border_brush);
    FillRect(dc, &layout.difficulty_card, card_brush);
    FrameRect(dc, &layout.difficulty_card, border_brush);
    FillRect(dc, &layout.action_card, card_brush);
    FrameRect(dc, &layout.action_card, border_brush);
    DeleteObject(card_brush);

    HFONT label_font = CreateFontW(ScaleByDpi(hwnd, 18), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                   CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    old_font = static_cast<HFONT>(SelectObject(dc, label_font));
    RECT label_rect = layout.first_card;
    label_rect.bottom = label_rect.top + ScaleByDpi(hwnd, 22);
    label_rect.top += ScaleByDpi(hwnd, 10);
    DrawTextW(dc, L"Who moves first", -1, &label_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DrawButton(dc, layout.player_first, L"Player", state.player_first);
    DrawButton(dc, layout.ai_first, L"AI", !state.player_first);

    label_rect = layout.difficulty_card;
    label_rect.bottom = label_rect.top + ScaleByDpi(hwnd, 22);
    label_rect.top += ScaleByDpi(hwnd, 10);
    DrawTextW(dc, L"AI Difficulty", -1, &label_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DrawButton(dc, layout.easy, L"Easy", state.difficulty == AiDifficulty::Easy);
    DrawButton(dc, layout.normal, L"Normal", state.difficulty == AiDifficulty::Normal);
    DrawButton(dc, layout.hard, L"Hard", state.difficulty == AiDifficulty::Hard);

    label_rect = layout.action_card;
    label_rect.bottom = label_rect.top + ScaleByDpi(hwnd, 22);
    label_rect.top += ScaleByDpi(hwnd, 10);
    DrawTextW(dc, L"Actions", -1, &label_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DrawButton(dc, layout.start, L"Start", false);
    DrawButton(dc, layout.quit, L"Quit", false);
    SelectObject(dc, old_font);
    DeleteObject(label_font);
}

void DrawGameOverOverlay(HDC dc, HWND hwnd, const RECT &client, const GameState &state) {
    RECT overlay = client;
    HBRUSH overlay_brush = CreateSolidBrush(RGB(245, 245, 245));
    FillRect(dc, &overlay, overlay_brush);
    DeleteObject(overlay_brush);

    if (!state.win_cells.empty() && state.win_blink_on) {
        POINT start = state.win_cells.front();
        POINT end = state.win_cells.back();
        int start_cx = kMargin + start.x * kCellSize;
        int start_cy = kMargin + start.y * kCellSize;
        int end_cx = kMargin + end.x * kCellSize;
        int end_cy = kMargin + end.y * kCellSize;

        HPEN win_pen = CreatePen(PS_SOLID, 6, RGB(200, 60, 60));
        HPEN old_win_pen = static_cast<HPEN>(SelectObject(dc, win_pen));
        MoveToEx(dc, start_cx, start_cy, nullptr);
        LineTo(dc, end_cx, end_cy);
        SelectObject(dc, old_win_pen);
        DeleteObject(win_pen);
    }

    const wchar_t *result_text = L"DRAW";
    if (state.winner == state.human_player) {
        result_text = L"YOU WIN!";
    } else if (state.winner == state.ai_player) {
        result_text = L"YOU LOSE!";
    }

    int panel_width = ScaleByDpi(hwnd, 420);
    int panel_height = ScaleByDpi(hwnd, 240);
    RECT panel = MakeCenteredRect(client, (client.top + client.bottom) / 2, panel_width, panel_height);
    HBRUSH panel_brush = CreateSolidBrush(RGB(250, 250, 250));
    FillRect(dc, &panel, panel_brush);
    DeleteObject(panel_brush);
    FrameRect(dc, &panel, static_cast<HBRUSH>(GetStockObject(GRAY_BRUSH)));

    HFONT big_font = CreateFontW(ScaleByDpi(hwnd, 44), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HFONT old_font = static_cast<HFONT>(SelectObject(dc, big_font));
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(40, 40, 40));
    RECT text_rect = MakeCenteredRect(client, panel.top + ScaleByDpi(hwnd, 40), panel_width - 40, ScaleByDpi(hwnd, 60));
    DrawTextW(dc, result_text, -1, &text_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, old_font);
    DeleteObject(big_font);

    GameOverLayout layout = BuildGameOverLayout(hwnd, client);
    DrawButton(dc, layout.play_again, L"Play Again", false);
    DrawButton(dc, layout.back_to_menu, L"Back to Menu", false);
}

bool HitTest(const RECT &rect, int x, int y) {
    POINT pt{x, y};
    return PtInRect(&rect, pt) != 0;
}

std::vector<POINT> BuildWinCells(const WinLine &line) {
    std::vector<POINT> cells;
    cells.reserve(5);
    for (int i = 0; i < 5; ++i) {
        cells.push_back(POINT{line.start_x + line.dx * i, line.start_y + line.dy * i});
    }
    return cells;
}

void StartBlinkTimer(GameState &state, HWND hwnd) {
    state.win_blink_on = true;
    KillTimer(hwnd, kBlinkTimerId);
    SetTimer(hwnd, kBlinkTimerId, kBlinkIntervalMs, nullptr);
}

void StartPulseTimer(GameState &state, HWND hwnd) {
    state.pulse_frame = 0;
    state.pulse_total = kPulseFrames;
    KillTimer(hwnd, kPulseTimerId);
    SetTimer(hwnd, kPulseTimerId, kPulseIntervalMs, nullptr);
}

void SetWinState(GameState &state, HWND hwnd, int winner, const std::optional<WinLine> &line) {
    state.winner = winner;
    state.win_line = line;
    state.win_cells.clear();
    if (line) {
        state.win_cells = BuildWinCells(*line);
        StartBlinkTimer(state, hwnd);
    }
    state.scene = Scene::GameOver;
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

    if (history.back().player == state.ai_player && history.size() >= 2) {
        state.game.undoLastMove();
        state.game.undoLastMove();
    } else {
        state.game.undoLastMove();
    }

    state.winner = GomokuGame::kEmpty;
    state.win_line.reset();
    state.win_cells.clear();
    state.win_blink_on = true;
    KillTimer(hwnd, kBlinkTimerId);
    InvalidateRect(hwnd, nullptr, TRUE);
}

static void DoAiMove(GameState &state, HWND hwnd) {
    state.ai_pending = false;
    if (state.scene != Scene::Playing || state.winner != GomokuGame::kEmpty || state.game.isBoardFull()) {
        return;
    }
    if (state.game.currentPlayer() != state.ai_player) {
        return;
    }
    auto move = ComputeAiMove(state.game, state.ai_player, state.human_player, state.difficulty);
    if (state.game.placeStone(move.first, move.second, state.ai_player)) {
        StartPulseTimer(state, hwnd);
        std::optional<WinLine> win_line = state.game.findWinningLine(move.first, move.second, state.ai_player);
        if (win_line) {
            SetWinState(state, hwnd, state.ai_player, win_line);
        } else if (state.game.isBoardFull()) {
            state.scene = Scene::GameOver;
        } else {
            state.game.setCurrentPlayer(state.human_player);
        }
    }
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
            if (!state) {
                return 0;
            }
            int mouse_x = GET_X_LPARAM(lparam);
            int mouse_y = GET_Y_LPARAM(lparam);
            RECT client;
            GetClientRect(hwnd, &client);
            if (state->scene == Scene::Menu) {
                MenuLayout layout = BuildMenuLayout(hwnd, client);
                if (HitTest(layout.player_first, mouse_x, mouse_y)) {
                    state->player_first = true;
                } else if (HitTest(layout.ai_first, mouse_x, mouse_y)) {
                    state->player_first = false;
                } else if (HitTest(layout.easy, mouse_x, mouse_y)) {
                    state->difficulty = AiDifficulty::Easy;
                } else if (HitTest(layout.normal, mouse_x, mouse_y)) {
                    state->difficulty = AiDifficulty::Normal;
                } else if (HitTest(layout.hard, mouse_x, mouse_y)) {
                    state->difficulty = AiDifficulty::Hard;
                } else if (HitTest(layout.start, mouse_x, mouse_y)) {
                    StartMatch(*state, hwnd);
                } else if (HitTest(layout.quit, mouse_x, mouse_y)) {
                    DestroyWindow(hwnd);
                    return 0;
                }
                InvalidateRect(hwnd, nullptr, TRUE);
                return 0;
            }
            if (state->scene == Scene::GameOver) {
                GameOverLayout layout = BuildGameOverLayout(hwnd, client);
                if (HitTest(layout.play_again, mouse_x, mouse_y)) {
                    StartMatch(*state, hwnd);
                } else if (HitTest(layout.back_to_menu, mouse_x, mouse_y)) {
                    ResetGame(*state, hwnd);
                    state->scene = Scene::Menu;
                }
                InvalidateRect(hwnd, nullptr, TRUE);
                return 0;
            }
            if (state->scene != Scene::Playing || state->winner != GomokuGame::kEmpty || state->ai_pending) {
                return 0;
            }
            if (state->game.currentPlayer() != state->human_player) {
                return 0;
            }
            int col = (mouse_x - kMargin + kCellSize / 2) / kCellSize;
            int row = (mouse_y - kMargin + kCellSize / 2) / kCellSize;
            if (col < 0 || col >= GomokuGame::kBoardSize || row < 0 || row >= GomokuGame::kBoardSize) {
                return 0;
            }
            if (!state->game.placeStone(col, row, state->human_player)) {
                return 0;
            }
            StartPulseTimer(*state, hwnd);
            std::optional<WinLine> win_line = state->game.findWinningLine(col, row, state->human_player);
            if (win_line) {
                SetWinState(*state, hwnd, state->human_player, win_line);
            } else if (state->game.isBoardFull()) {
                state->scene = Scene::GameOver;
            } else {
                state->game.setCurrentPlayer(state->ai_player);
                state->ai_pending = true;
                StartAiTimer(hwnd);
            }
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        case WM_TIMER: {
            if (!state) {
                return 0;
            }
            if (wparam == kAiTimerId) {
                KillTimer(hwnd, kAiTimerId);
                DoAiMove(*state, hwnd);
                InvalidateRect(hwnd, nullptr, FALSE);
                return 0;
            }
            if (wparam == kBlinkTimerId) {
                state->win_blink_on = !state->win_blink_on;
                InvalidateRect(hwnd, nullptr, FALSE);
                return 0;
            }
            if (wparam == kPulseTimerId) {
                state->pulse_frame++;
                if (state->pulse_frame >= state->pulse_total) {
                    state->pulse_total = 0;
                    KillTimer(hwnd, kPulseTimerId);
                }
                InvalidateRect(hwnd, nullptr, FALSE);
                return 0;
            }
            return 0;
        }
        case WM_KEYDOWN: {
            if (!state) {
                return 0;
            }
            if (wparam == VK_ESCAPE) {
                ResetGame(*state, hwnd);
                state->scene = Scene::Menu;
                return 0;
            }
            if (wparam == 'R') {
                StartMatch(*state, hwnd);
                return 0;
            }
            if (wparam == 'U') {
                if (state->scene == Scene::Playing) {
                    HandleUndo(*state, hwnd);
                }
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
            if (state->scene == Scene::Menu) {
                DrawMenu(dc, hwnd, client, *state);
            } else {
                DrawBoardBase(dc, client, *state);
                if (state->scene == Scene::GameOver) {
                    DrawGameOverOverlay(dc, hwnd, client, *state);
                }
            }
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

    UINT dpi = GetDpiForSystem();
    int client_width = ScaleByDpi(dpi, kWindowWidth);
    int client_height = ScaleByDpi(dpi, kWindowHeight);

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
