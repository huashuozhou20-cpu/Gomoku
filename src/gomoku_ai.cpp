#include "gomoku_ai.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <random>
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

std::vector<std::pair<int, int>> SelectTopCandidates(const GomokuGame &game, int player, int limit) {
    auto candidates = GenerateCandidates(game);
    struct ScoredMove {
        std::pair<int, int> move;
        int score = 0;
    };
    std::vector<ScoredMove> scored;
    scored.reserve(candidates.size());

    for (const auto &move : candidates) {
        int x = move.first;
        int y = move.second;
        int score = EvaluateCell(game, x, y, player);
        int center_bias = std::abs(x - GomokuGame::kBoardSize / 2)
                        + std::abs(y - GomokuGame::kBoardSize / 2);
        score -= center_bias * 3;
        score += ProximityScore(game, x, y);
        scored.push_back({move, score});
    }

    std::sort(scored.begin(), scored.end(), [](const ScoredMove &a, const ScoredMove &b) {
        return a.score > b.score;
    });

    std::vector<std::pair<int, int>> top_moves;
    int count = 0;
    for (const auto &entry : scored) {
        top_moves.push_back(entry.move);
        ++count;
        if (count >= limit) {
            break;
        }
    }
    if (top_moves.empty() && !candidates.empty()) {
        top_moves.push_back(candidates.front());
    }
    return top_moves;
}

int EvaluateBoard(const GomokuGame &game, int ai_player, int human_player) {
    int score = 0;
    auto candidates = GenerateCandidates(game);
    for (const auto &move : candidates) {
        int x = move.first;
        int y = move.second;
        score += EvaluateCell(game, x, y, ai_player);
        score -= EvaluateCell(game, x, y, human_player);
    }
    return score;
}

int Minimax(GomokuGame &game, int depth, bool maximizing, int ai_player, int human_player, int alpha, int beta) {
    if (depth == 0 || game.isBoardFull()) {
        return EvaluateBoard(game, ai_player, human_player);
    }

    int player = maximizing ? ai_player : human_player;
    auto candidates = SelectTopCandidates(game, player, 8);
    if (candidates.empty()) {
        return EvaluateBoard(game, ai_player, human_player);
    }

    if (maximizing) {
        int best = std::numeric_limits<int>::min();
        for (const auto &move : candidates) {
            if (!game.placeStone(move.first, move.second, player)) {
                continue;
            }
            int score = 0;
            if (game.findWinningLine(move.first, move.second, player)) {
                score = 1000000 + depth * 100;
            } else {
                score = Minimax(game, depth - 1, false, ai_player, human_player, alpha, beta);
            }
            game.undoLastMove();
            if (score > best) {
                best = score;
            }
            if (best > alpha) {
                alpha = best;
            }
            if (beta <= alpha) {
                break;
            }
        }
        return best;
    }

    int best = std::numeric_limits<int>::max();
    for (const auto &move : candidates) {
        if (!game.placeStone(move.first, move.second, player)) {
            continue;
        }
        int score = 0;
        if (game.findWinningLine(move.first, move.second, player)) {
            score = -1000000 - depth * 100;
        } else {
            score = Minimax(game, depth - 1, true, ai_player, human_player, alpha, beta);
        }
        game.undoLastMove();
        if (score < best) {
            best = score;
        }
        if (best < beta) {
            beta = best;
        }
        if (beta <= alpha) {
            break;
        }
    }
    return best;
}

} // namespace

std::pair<int, int> ComputeAiMove(const GomokuGame &game, int ai_player, int human_player, AiDifficulty difficulty) {
    auto candidates = GenerateCandidates(game);

    if (difficulty == AiDifficulty::Easy) {
        for (const auto &move : candidates) {
            if (WouldWin(game, move.first, move.second, ai_player)) {
                return move;
            }
        }
        for (const auto &move : candidates) {
            if (WouldWin(game, move.first, move.second, human_player)) {
                return move;
            }
        }

        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
        return candidates[dist(rng)];
    }

    if (difficulty == AiDifficulty::Hard) {
        GomokuGame copy = game;
        auto top_moves = SelectTopCandidates(copy, ai_player, 8);
        int best_score = std::numeric_limits<int>::min();
        std::pair<int, int> best_move = top_moves.front();
        for (const auto &move : top_moves) {
            if (!copy.placeStone(move.first, move.second, ai_player)) {
                continue;
            }
            int score = 0;
            if (copy.findWinningLine(move.first, move.second, ai_player)) {
                score = 1000000;
            } else {
                score = Minimax(copy, 2, false, ai_player, human_player,
                                std::numeric_limits<int>::min(),
                                std::numeric_limits<int>::max());
            }
            copy.undoLastMove();
            if (score > best_score) {
                best_score = score;
                best_move = move;
            }
        }
        return best_move;
    }

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
