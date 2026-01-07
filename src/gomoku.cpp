#include "gomoku.h"

GomokuGame::GomokuGame() {
    reset();
}

void GomokuGame::reset() {
    for (auto &row : board_) {
        row.fill(kEmpty);
    }
    current_player_ = kBlack;
    last_move_.reset();
    moves_.clear();
}

bool GomokuGame::isInside(int x, int y) const {
    return x >= 0 && x < kBoardSize && y >= 0 && y < kBoardSize;
}

bool GomokuGame::placeStone(int x, int y, int player) {
    if (!isInside(x, y) || board_[y][x] != kEmpty) {
        return false;
    }
    board_[y][x] = player;
    Move move{x, y, player};
    moves_.push_back(move);
    last_move_ = move;
    return true;
}

bool GomokuGame::undoLastMove() {
    if (moves_.empty()) {
        return false;
    }
    Move move = moves_.back();
    moves_.pop_back();
    board_[move.y][move.x] = kEmpty;
    current_player_ = move.player;
    if (moves_.empty()) {
        last_move_.reset();
    } else {
        last_move_ = moves_.back();
    }
    return true;
}

bool GomokuGame::checkWin(int x, int y, int player) const {
    return findWinningLine(x, y, player).has_value();
}

std::optional<WinLine> GomokuGame::findWinningLine(int x, int y, int player) const {
    const int directions[4][2] = {
        {1, 0}, {0, 1}, {1, 1}, {1, -1}
    };

    for (const auto &dir : directions) {
        int count = 1;
        int count_pos = 0;
        int count_neg = 0;
        for (int step = 1; step < 5; ++step) {
            int nx = x + dir[0] * step;
            int ny = y + dir[1] * step;
            if (!isInside(nx, ny) || board_[ny][nx] != player) {
                break;
            }
            ++count;
            ++count_pos;
        }
        for (int step = 1; step < 5; ++step) {
            int nx = x - dir[0] * step;
            int ny = y - dir[1] * step;
            if (!isInside(nx, ny) || board_[ny][nx] != player) {
                break;
            }
            ++count;
            ++count_neg;
        }
        if (count >= 5) {
            int total = count_pos + count_neg + 1;
            int offset_from_start = count_neg;
            int start_offset = offset_from_start - 4;
            if (start_offset < 0) {
                start_offset = 0;
            }
            if (start_offset > total - 5) {
                start_offset = total - 5;
            }
            int start_x = x - dir[0] * count_neg + dir[0] * start_offset;
            int start_y = y - dir[1] * count_neg + dir[1] * start_offset;
            return WinLine{start_x, start_y, dir[0], dir[1], 5};
        }
    }

    return std::nullopt;
}

bool GomokuGame::isBoardFull() const {
    for (const auto &row : board_) {
        for (int cell : row) {
            if (cell == kEmpty) {
                return false;
            }
        }
    }
    return true;
}
