#pragma once

#include "board.h"
#include "eval.h"
#include "movegenerator.h"
#include <vector>
#include <chrono>

class Search {
public:
    static uint64_t nodes_searched;
    static int max_depth_reached;
    
    static Move get_best_move(Board& board, int max_depth, int time_limit_ms);
    
    static void stop_search();
    static void set_time_limit(int time_limit_ms);
    
    static int negamax(Board& board, int depth, int alpha, int beta, int ply = 0);
    static int quiescence(Board& board, int alpha, int beta);
    static void reset_stats();

private:
    static constexpr int MAX_DEPTH = 64;
    static constexpr int MAX_KILLER_MOVES = 2;
    
    static std::chrono::steady_clock::time_point start_time;
    static int allotted_time_ms;
    static bool stop_flag;
    static bool is_time_up();

    // Déclaration des structures de données statiques (définies dans le .cpp)
    static Move killer_moves[MAX_DEPTH][MAX_KILLER_MOVES];
    static int history[12][64];
    
    // Signature corrigée avec tous les arguments nécessaires
    static int score_move(Move move, const Board& board, int ply, Move tt_move);
};