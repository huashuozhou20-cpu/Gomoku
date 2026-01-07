#include "game.h"

#include <SDL.h>

#include <cmath>
#include <string>

namespace {
constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 860;
constexpr int kBoardOffsetX = 60;
constexpr int kBoardOffsetY = 120;
constexpr int kCellSize = 40;
constexpr int kBoardPixelSize = kCellSize * Game::kBoardSize;

bool IsPointInBoard(int x, int y) {
    return x >= kBoardOffsetX && x <= kBoardOffsetX + kBoardPixelSize &&
           y >= kBoardOffsetY && y <= kBoardOffsetY + kBoardPixelSize;
}

std::pair<int, int> ScreenToCell(int x, int y) {
    int cell_x = (x - kBoardOffsetX) / kCellSize;
    int cell_y = (y - kBoardOffsetY) / kCellSize;
    return {cell_x, cell_y};
}

void DrawFilledCircle(SDL_Renderer *renderer, int center_x, int center_y, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int dy = -radius; dy <= radius; ++dy) {
        int dx_limit = static_cast<int>(std::sqrt(radius * radius - dy * dy));
        SDL_RenderDrawLine(renderer, center_x - dx_limit, center_y + dy,
                           center_x + dx_limit, center_y + dy);
    }
}
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Gomoku", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          kWindowWidth, kWindowHeight, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Game game;
    bool game_over = false;
    int winner = 0;

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {
                game.reset();
                game_over = false;
                winner = 0;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouse_x = event.button.x;
                int mouse_y = event.button.y;
                if (!game_over && game.currentPlayer() == 1 && IsPointInBoard(mouse_x, mouse_y)) {
                    auto [x, y] = ScreenToCell(mouse_x, mouse_y);
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

        std::string title = "Gomoku";
        if (game_over) {
            if (winner == 1) {
                title += " - Human wins";
            } else if (winner == 2) {
                title += " - AI wins";
            } else {
                title += " - Draw";
            }
        } else {
            title += game.currentPlayer() == 1 ? " - Turn: Human" : " - Turn: AI";
        }
        SDL_SetWindowTitle(window, title.c_str());

        SDL_SetRenderDrawColor(renderer, 245, 235, 215, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 90, 60, 30, 255);
        for (int i = 0; i <= Game::kBoardSize; ++i) {
            int start_x = kBoardOffsetX;
            int start_y = kBoardOffsetY + i * kCellSize;
            int end_x = kBoardOffsetX + kBoardPixelSize;
            SDL_RenderDrawLine(renderer, start_x, start_y, end_x, start_y);
        }
        for (int i = 0; i <= Game::kBoardSize; ++i) {
            int start_x = kBoardOffsetX + i * kCellSize;
            int start_y = kBoardOffsetY;
            int end_y = kBoardOffsetY + kBoardPixelSize;
            SDL_RenderDrawLine(renderer, start_x, start_y, start_x, end_y);
        }

        for (int y = 0; y < Game::kBoardSize; ++y) {
            for (int x = 0; x < Game::kBoardSize; ++x) {
                int value = game.at(x, y);
                if (value == 0) {
                    continue;
                }
                int center_x = kBoardOffsetX + x * kCellSize + kCellSize / 2;
                int center_y = kBoardOffsetY + y * kCellSize + kCellSize / 2;
                SDL_Color color = value == 1 ? SDL_Color{40, 40, 40, 255} : SDL_Color{230, 230, 230, 255};
                DrawFilledCircle(renderer, center_x, center_y, 16, color);
            }
        }

        if (auto last = game.lastMove()) {
            int x = last->first;
            int y = last->second;
            SDL_Rect rect{ kBoardOffsetX + x * kCellSize + 2,
                           kBoardOffsetY + y * kCellSize + 2,
                           kCellSize - 4, kCellSize - 4 };
            SDL_SetRenderDrawColor(renderer, 200, 40, 40, 255);
            SDL_RenderDrawRect(renderer, &rect);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
