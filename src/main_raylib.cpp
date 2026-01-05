#include "game.h"

#include "raylib.h"

#include <string>

namespace {
constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 860;
constexpr int kBoardOffsetX = 60;
constexpr int kBoardOffsetY = 120;
constexpr int kCellSize = 40;
constexpr int kBoardPixelSize = kCellSize * Game::kBoardSize;

struct Button {
    Rectangle rect;
    const char *label;
};

bool IsPointInBoard(Vector2 point) {
    return point.x >= kBoardOffsetX && point.x <= kBoardOffsetX + kBoardPixelSize &&
           point.y >= kBoardOffsetY && point.y <= kBoardOffsetY + kBoardPixelSize;
}

std::pair<int, int> ScreenToCell(Vector2 point) {
    int x = static_cast<int>((point.x - kBoardOffsetX) / kCellSize);
    int y = static_cast<int>((point.y - kBoardOffsetY) / kCellSize);
    return {x, y};
}
}

int main() {
    InitWindow(kWindowWidth, kWindowHeight, "Gomoku");
    SetTargetFPS(60);

    Game game;
    bool game_over = false;
    int winner = 0;

    Button restart_button{{kWindowWidth - 160.0f, 40.0f, 120.0f, 40.0f}, "Restart"};

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) {
            game.reset();
            game_over = false;
            winner = 0;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();

            if (CheckCollisionPointRec(mouse, restart_button.rect)) {
                game.reset();
                game_over = false;
                winner = 0;
            } else if (!game_over && IsPointInBoard(mouse) && game.currentPlayer() == 1) {
                auto [x, y] = ScreenToCell(mouse);
                if (game.placeStone(x, y, 1)) {
                    if (game.checkWin(x, y, 1)) {
                        game_over = true;
                        winner = 1;
                    } else if (game.isBoardFull()) {
                        game_over = true;
                        winner = 0;
                    } else {
                        game.setCurrentPlayer(2);
                    }
                }
            }
        }

        if (!game_over && game.currentPlayer() == 2) {
            auto [x, y] = game.computeAIMove();
            if (game.placeStone(x, y, 2)) {
                if (game.checkWin(x, y, 2)) {
                    game_over = true;
                    winner = 2;
                } else if (game.isBoardFull()) {
                    game_over = true;
                    winner = 0;
                } else {
                    game.setCurrentPlayer(1);
                }
            }
        }

        BeginDrawing();
        ClearBackground({245, 235, 215, 255});

        DrawText("Gomoku", 60, 30, 32, DARKBROWN);
        DrawText("Click to place stones. Press R to restart.", 60, 70, 18, DARKGRAY);

        Color button_color = CheckCollisionPointRec(GetMousePosition(), restart_button.rect)
                                 ? Color{200, 150, 120, 255}
                                 : Color{220, 180, 150, 255};
        DrawRectangleRec(restart_button.rect, button_color);
        DrawRectangleLinesEx(restart_button.rect, 2, DARKBROWN);
        DrawText(restart_button.label, static_cast<int>(restart_button.rect.x) + 20,
                 static_cast<int>(restart_button.rect.y) + 10, 20, DARKBROWN);

        if (game_over) {
            const char *result = "Draw";
            if (winner == 1) {
                result = "Human wins!";
            } else if (winner == 2) {
                result = "AI wins!";
            }
            DrawText(result, 420, 70, 20, MAROON);
        } else {
            const char *turn = game.currentPlayer() == 1 ? "Human" : "AI";
            std::string turn_text = std::string("Turn: ") + turn;
            DrawText(turn_text.c_str(), 420, 70, 20, DARKGREEN);
        }

        for (int i = 0; i <= Game::kBoardSize; ++i) {
            int start_x = kBoardOffsetX;
            int start_y = kBoardOffsetY + i * kCellSize;
            int end_x = kBoardOffsetX + kBoardPixelSize;
            DrawLine(start_x, start_y, end_x, start_y, DARKBROWN);
        }
        for (int i = 0; i <= Game::kBoardSize; ++i) {
            int start_x = kBoardOffsetX + i * kCellSize;
            int start_y = kBoardOffsetY;
            int end_y = kBoardOffsetY + kBoardPixelSize;
            DrawLine(start_x, start_y, start_x, end_y, DARKBROWN);
        }

        for (int y = 0; y < Game::kBoardSize; ++y) {
            for (int x = 0; x < Game::kBoardSize; ++x) {
                int value = game.at(x, y);
                if (value == 0) {
                    continue;
                }
                int center_x = kBoardOffsetX + x * kCellSize + kCellSize / 2;
                int center_y = kBoardOffsetY + y * kCellSize + kCellSize / 2;
                Color stone_color = value == 1 ? Color{40, 40, 40, 255} : Color{230, 230, 230, 255};
                DrawCircle(center_x, center_y, 16, stone_color);
                DrawCircleLines(center_x, center_y, 16, DARKBROWN);
            }
        }

        if (auto last = game.lastMove()) {
            int x = last->first;
            int y = last->second;
            int rect_x = kBoardOffsetX + x * kCellSize + 2;
            int rect_y = kBoardOffsetY + y * kCellSize + 2;
            DrawRectangleLines(rect_x, rect_y, kCellSize - 4, kCellSize - 4, RED);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
