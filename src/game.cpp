#include "game.h"

#include <algorithm>
#include <cmath>
#include <random>

Game::Game() {
    reset();
}

void Game::reset() {
    for (auto &row : board_) {
        row.fill(0);
    }
    current_player_ = 1;
    last_move_.reset();
}

bool Game::isInside(int x, int y) const {
    return x >= 0 && x < kBoardSize && y >= 0 && y < kBoardSize;
}

bool Game::placeStone(int x, int y, int player) {
    if (!isInside(x, y) || board_[y][x] != 0) {
        return false;
    }
    board_[y][x] = player;
    last_move_ = std::make_pair(x, y);
    return true;
}

bool Game::checkWin(int x, int y, int player) const {
    const int directions[4][2] = {
        {1, 0}, {0, 1}, {1, 1}, {1, -1}
    };

    for (const auto &dir : directions) {
        int count = 1;
        for (int step = 1; step < 5; ++step) {
            int nx = x + dir[0] * step;
            int ny = y + dir[1] * step;
            if (!isInside(nx, ny) || board_[ny][nx] != player) {
                break;
            }
            ++count;
        }
        for (int step = 1; step < 5; ++step) {
            int nx = x - dir[0] * step;
            int ny = y - dir[1] * step;
            if (!isInside(nx, ny) || board_[ny][nx] != player) {
                break;
            }
            ++count;
        }
        if (count >= 5) {
            return true;
        }
    }

    return false;
}

bool Game::isBoardFull() const {
    for (const auto &row : board_) {
        for (int cell : row) {
            if (cell == 0) {
                return false;
            }
        }
    }
    return true;
}

int Game::evaluateDirection(int x, int y, int player, int dx, int dy) const {
    int count_pos = 0;
    int count_neg = 0;

    int nx = x + dx;
    int ny = y + dy;
    while (isInside(nx, ny) && board_[ny][nx] == player) {
        ++count_pos;
        nx += dx;
        ny += dy;
    }
    bool open_pos = isInside(nx, ny) && board_[ny][nx] == 0;

    nx = x - dx;
    ny = y - dy;
    while (isInside(nx, ny) && board_[ny][nx] == player) {
        ++count_neg;
        nx -= dx;
        ny -= dy;
    }
    bool open_neg = isInside(nx, ny) && board_[ny][nx] == 0;

    int total = count_pos + count_neg + 1;
    int open_ends = static_cast<int>(open_pos) + static_cast<int>(open_neg);

    if (total >= 5) {
        return 1000000;
    }
    if (total == 4 && open_ends == 2) {
        return 100000;
    }
    if (total == 4 && open_ends == 1) {
        return 15000;
    }
    if (total == 3 && open_ends == 2) {
        return 4000;
    }
    if (total == 3 && open_ends == 1) {
        return 400;
    }
    if (total == 2 && open_ends == 2) {
        return 120;
    }
    if (total == 2 && open_ends == 1) {
        return 20;
    }
    if (open_ends == 2) {
        return 10;
    }
    return 2;
}

int Game::evaluateCell(int x, int y, int player) const {
    const int directions[4][2] = {
        {1, 0}, {0, 1}, {1, 1}, {1, -1}
    };
    int score = 0;
    for (const auto &dir : directions) {
        score += evaluateDirection(x, y, player, dir[0], dir[1]);
    }
    return score;
}

std::vector<std::pair<int, int>> Game::generateCandidates() const {
    std::vector<std::pair<int, int>> candidates;
    bool hasStone = false;

    for (int y = 0; y < kBoardSize; ++y) {
        for (int x = 0; x < kBoardSize; ++x) {
            if (board_[y][x] != 0) {
                hasStone = true;
            }
        }
    }

    if (!hasStone) {
        return { {kBoardSize / 2, kBoardSize / 2} };
    }

    std::array<std::array<bool, kBoardSize>, kBoardSize> marked{};
    for (int y = 0; y < kBoardSize; ++y) {
        for (int x = 0; x < kBoardSize; ++x) {
            if (board_[y][x] == 0) {
                continue;
            }
            for (int dy = -2; dy <= 2; ++dy) {
                for (int dx = -2; dx <= 2; ++dx) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (!isInside(nx, ny) || board_[ny][nx] != 0) {
                        continue;
                    }
                    if (!marked[ny][nx]) {
                        marked[ny][nx] = true;
                        candidates.emplace_back(nx, ny);
                    }
                }
            }
        }
    }

    if (candidates.empty()) {
        candidates.emplace_back(kBoardSize / 2, kBoardSize / 2);
    }

    return candidates;
}

std::pair<int, int> Game::computeAIMove() {
    const int ai_player = 2;
    const int human_player = 1;

    auto candidates = generateCandidates();

    int best_score = -1;
    std::pair<int, int> best_move = candidates.front();

    for (const auto &move : candidates) {
        int x = move.first;
        int y = move.second;
        int ai_score = evaluateCell(x, y, ai_player);
        int human_score = evaluateCell(x, y, human_player);
        int score = ai_score + static_cast<int>(human_score * 0.9);

        int center_bias = static_cast<int>(std::abs(x - kBoardSize / 2)
                                         + std::abs(y - kBoardSize / 2));
        score -= center_bias * 3;

        if (score > best_score) {
            best_score = score;
            best_move = move;
        }
    }

    return best_move;
}
