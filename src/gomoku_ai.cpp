#include "gomoku_ai.h"

#include <array>
#include <cmath>
#include <vector>

namespace {

bool IsInside(int x, int y) {
    return x >= 0 && x < GomokuGame::kBoardSize && y >= 0 && y < GomokuGame::kBoardSize;
}

bool WouldWin(const GomokuGame &game, int x, int y, int player) {
    const int directions[4][2] = {
        {1, 0}, {0, 1}, {1, 1}, {1, -1}
    };

    for (const auto &dir : directions) {
        int count = 1;
        for (int step = 1; step < 5; ++step) {
            int nx = x + dir[0] * step;
            int ny = y + dir[1] * step;
            if (!IsInside(nx, ny) || game.at(nx, ny) != player) {
                break;
            }
            ++count;
        }
        for (int step = 1; step < 5; ++step) {
            int nx = x - dir[0] * step;
            int ny = y - dir[1] * step;
            if (!IsInside(nx, ny) || game.at(nx, ny) != player) {
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

int EvaluateDirection(const GomokuGame &game, int x, int y, int player, int dx, int dy) {
    int count_pos = 0;
    int count_neg = 0;

    int nx = x + dx;
    int ny = y + dy;
    while (IsInside(nx, ny) && game.at(nx, ny) == player) {
        ++count_pos;
        nx += dx;
        ny += dy;
    }
    bool open_pos = IsInside(nx, ny) && game.at(nx, ny) == GomokuGame::kEmpty;

    nx = x - dx;
    ny = y - dy;
    while (IsInside(nx, ny) && game.at(nx, ny) == player) {
        ++count_neg;
        nx -= dx;
        ny -= dy;
    }
    bool open_neg = IsInside(nx, ny) && game.at(nx, ny) == GomokuGame::kEmpty;

    int total = count_pos + count_neg + 1;
    int open_ends = static_cast<int>(open_pos) + static_cast<int>(open_neg);

    if (total >= 5) {
        return 1000000;
    }
    if (total == 4 && open_ends == 2) {
        return 120000;
    }
    if (total == 4 && open_ends == 1) {
        return 20000;
    }
    if (total == 3 && open_ends == 2) {
        return 8000;
    }
    if (total == 3 && open_ends == 1) {
        return 800;
    }
    if (total == 2 && open_ends == 2) {
        return 200;
    }
    if (total == 2 && open_ends == 1) {
        return 50;
    }
    if (open_ends == 2) {
        return 10;
    }
    return 2;
}

int EvaluateCell(const GomokuGame &game, int x, int y, int player) {
    const int directions[4][2] = {
        {1, 0}, {0, 1}, {1, 1}, {1, -1}
    };
    int score = 0;
    for (const auto &dir : directions) {
        score += EvaluateDirection(game, x, y, player, dir[0], dir[1]);
    }
    return score;
}

std::vector<std::pair<int, int>> GenerateCandidates(const GomokuGame &game) {
    std::vector<std::pair<int, int>> candidates;
    bool hasStone = false;

    for (int y = 0; y < GomokuGame::kBoardSize; ++y) {
        for (int x = 0; x < GomokuGame::kBoardSize; ++x) {
            if (game.at(x, y) != GomokuGame::kEmpty) {
                hasStone = true;
            }
        }
    }

    if (!hasStone) {
        return { {GomokuGame::kBoardSize / 2, GomokuGame::kBoardSize / 2} };
    }

    std::array<std::array<bool, GomokuGame::kBoardSize>, GomokuGame::kBoardSize> marked{};
    for (int y = 0; y < GomokuGame::kBoardSize; ++y) {
        for (int x = 0; x < GomokuGame::kBoardSize; ++x) {
            if (game.at(x, y) == GomokuGame::kEmpty) {
                continue;
            }
            for (int dy = -2; dy <= 2; ++dy) {
                for (int dx = -2; dx <= 2; ++dx) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (!IsInside(nx, ny) || game.at(nx, ny) != GomokuGame::kEmpty) {
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
        candidates.emplace_back(GomokuGame::kBoardSize / 2, GomokuGame::kBoardSize / 2);
    }

    return candidates;
}

int ProximityScore(const GomokuGame &game, int x, int y) {
    int min_distance = 1000;
    for (int row = 0; row < GomokuGame::kBoardSize; ++row) {
        for (int col = 0; col < GomokuGame::kBoardSize; ++col) {
            if (game.at(col, row) == GomokuGame::kEmpty) {
                continue;
            }
            int distance = std::abs(col - x) + std::abs(row - y);
            if (distance < min_distance) {
                min_distance = distance;
            }
        }
    }
    if (min_distance == 1000) {
        return 0;
    }
    return 30 - min_distance * 2;
}

} // namespace

std::pair<int, int> ComputeAiMove(const GomokuGame &game) {
    const int ai_player = GomokuGame::kWhite;
    const int human_player = GomokuGame::kBlack;

    auto candidates = GenerateCandidates(game);

    for (const auto &move : candidates) {
        if (WouldWin(game, move.first, move.second, ai_player)) {
            return move;
        }
    }

    std::pair<int, int> best_block = candidates.front();
    int best_block_score = -1;
    for (const auto &move : candidates) {
        if (WouldWin(game, move.first, move.second, human_player)) {
            int score = EvaluateCell(game, move.first, move.second, human_player);
            if (score > best_block_score) {
                best_block_score = score;
                best_block = move;
            }
        }
    }
    if (best_block_score >= 0) {
        return best_block;
    }

    int best_score = -1;
    std::pair<int, int> best_move = candidates.front();

    for (const auto &move : candidates) {
        int x = move.first;
        int y = move.second;
        int ai_score = EvaluateCell(game, x, y, ai_player);
        int human_score = EvaluateCell(game, x, y, human_player);
        int score = static_cast<int>(ai_score * 1.2 + human_score);

        int center_bias = std::abs(x - GomokuGame::kBoardSize / 2)
                        + std::abs(y - GomokuGame::kBoardSize / 2);
        score -= center_bias * 3;
        score += ProximityScore(game, x, y);

        if (score > best_score) {
            best_score = score;
            best_move = move;
        }
    }

    return best_move;
}
