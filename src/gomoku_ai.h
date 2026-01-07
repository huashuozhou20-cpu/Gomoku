#ifndef GOMOKU_GOMOKU_AI_H
#define GOMOKU_GOMOKU_AI_H

#include <utility>

#include "gomoku.h"

enum class AiDifficulty {
    Easy,
    Normal,
    Hard
};

std::pair<int, int> ComputeAiMove(const GomokuGame &game, int ai_player, int human_player, AiDifficulty difficulty);

#endif
