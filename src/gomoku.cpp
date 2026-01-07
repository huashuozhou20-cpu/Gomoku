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
