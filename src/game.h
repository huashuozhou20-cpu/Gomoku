#ifndef GOMOKU_GAME_H
#define GOMOKU_GAME_H

#include <array>
#include <optional>
#include <utility>
#include <vector>

class Game {
public:
    static constexpr int kBoardSize = 15;

    Game();

    void reset();
    bool placeStone(int x, int y, int player);
    bool checkWin(int x, int y, int player) const;
    bool isBoardFull() const;

    std::pair<int, int> computeAIMove();

    int currentPlayer() const { return current_player_; }
    void setCurrentPlayer(int player) { current_player_ = player; }

    int at(int x, int y) const { return board_[y][x]; }

    std::optional<std::pair<int, int>> lastMove() const { return last_move_; }
    void setLastMove(int x, int y) { last_move_ = std::make_pair(x, y); }

private:
    std::array<std::array<int, kBoardSize>, kBoardSize> board_{};
    int current_player_ = 1;
    std::optional<std::pair<int, int>> last_move_{};

    bool isInside(int x, int y) const;
    int evaluateCell(int x, int y, int player) const;
    int evaluateDirection(int x, int y, int player, int dx, int dy) const;
    std::vector<std::pair<int, int>> generateCandidates() const;
};

#endif
