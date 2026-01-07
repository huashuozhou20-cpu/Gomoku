#ifndef GOMOKU_GOMOKU_H
#define GOMOKU_GOMOKU_H

#include <array>
#include <optional>
#include <vector>

struct Move {
    int x = 0;
    int y = 0;
    int player = 0;
};

struct WinLine {
    int start_x = 0;
    int start_y = 0;
    int dx = 0;
    int dy = 0;
    int length = 0;
};

class GomokuGame {
public:
    static constexpr int kBoardSize = 15;
    static constexpr int kEmpty = 0;
    static constexpr int kBlack = 1;
    static constexpr int kWhite = 2;

    GomokuGame();

    void reset();
    bool placeStone(int x, int y, int player);
    bool undoLastMove();
    bool checkWin(int x, int y, int player) const;
    std::optional<WinLine> findWinningLine(int x, int y, int player) const;
    bool isBoardFull() const;

    int at(int x, int y) const { return board_[y][x]; }
    int currentPlayer() const { return current_player_; }
    void setCurrentPlayer(int player) { current_player_ = player; }

    std::optional<Move> lastMove() const { return last_move_; }
    const std::vector<Move>& moveHistory() const { return moves_; }

private:
    std::array<std::array<int, kBoardSize>, kBoardSize> board_{};
    int current_player_ = kBlack;
    std::optional<Move> last_move_{};
    std::vector<Move> moves_{};

    bool isInside(int x, int y) const;
};

#endif
