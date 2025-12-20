#pragma once

#include "board.h"
#include "eval.h"
#include "movegenerator.h"
#include <vector>
#include <string>

class Search {
public:
    // Search statistics
    static uint64_t nodes_searched;
    static int max_depth_reached;
    
    // Get best move at given depth
    static Move get_best_move(Board& board, int depth);
    
    // Negamax with alpha-beta pruning
    static int negamax(Board& board, int depth, int alpha, int beta);
    
    // Quiescence search (only captures)
    static int quiescence(Board& board, int alpha, int beta);
    
    // Reset statistics
    static void reset_stats();
    
private:
    // Move ordering: score moves for better pruning
    static int score_move(Move move, const Board& board);
    static void order_moves(std::vector<Move>& moves, const Board& board);
};

