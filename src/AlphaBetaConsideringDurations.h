#pragma once

#include "AlphaBetaState.h"
#include "AlphaBetaValue.h"
#include "AlphaBetaMove.h"
#include "CCBot.h"

class AlphaBetaConsideringDurations {
    size_t time_limit;
    size_t actual_time;
    size_t depth_limit;
    bool closestEnemy;
    bool weakestEnemy;
    bool highestPriority;
    AlphaBetaValue alphaBeta(AlphaBetaState state, size_t depth, AlphaBetaMove * m0, AlphaBetaValue alpha, AlphaBetaValue beta);

public:
    size_t nodes_evaluated;
    AlphaBetaConsideringDurations(size_t time, size_t depth, bool pClosestEnemy, bool pWeakestEnemy, bool pHighestPriority);
    AlphaBetaValue doSearch(std::vector<std::shared_ptr<AlphaBetaUnit>> units, std::vector<std::shared_ptr<AlphaBetaUnit>> targets, CCBot * bot);

};