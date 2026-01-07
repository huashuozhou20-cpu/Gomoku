#ifndef GOMOKU_GOMOKU_AI_H
#define GOMOKU_GOMOKU_AI_H

#include <utility>

#include "gomoku.h"

std::pair<int, int> ComputeAiMove(const GomokuGame &game);

#endif
