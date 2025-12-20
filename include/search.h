#pragma once

#include "board.h"
#include "eval.h"
#include "movegenerator.h"
#include <vector>
#include <string>
#include <chrono>

class Search {
public:
    // Search statistics
    static uint64_t nodes_searched;
    static int max_depth_reached;
    
    // Get best move with iterative deepening and time management
    static Move get_best_move(Board& board, int max_depth, int time_limit_ms);
    
    // Negamax with alpha-beta pruning
    // ply: distance from root (for mate score adjustment)
    static int negamax(Board& board, int depth, int alpha, int beta, int ply = 0);
    
    // Quiescence search (only captures)
    static int quiescence(Board& board, int alpha, int beta);
    
    // Reset statistics and search state
    static void reset_stats();
    
    // Time management
    static void set_time_limit(int time_limit_ms);
    static bool is_time_up();
    static void stop_search();  // Force stop (for UCI stop command)
    
private:
    // Constants
    static constexpr int MAX_DEPTH = 64;
    static constexpr int MAX_KILLER_MOVES = 2;
    
    // Time management
    static std::chrono::steady_clock::time_point start_time;
    static int allotted_time_ms;
    static bool stop_flag;
    
    // Killer moves: moves that caused beta cutoffs at each depth
    // [depth][slot] - store up to 2 killer moves per depth
    static Move killer_moves[MAX_DEPTH][MAX_KILLER_MOVES];
    
    // History heuristic: [piece][to_square] - tracks successful quiet moves
    static int history[12][64];
    
    // Move ordering: score moves for better pruning
    static int score_move(Move move, const Board& board, int depth);
    static void order_moves(std::vector<Move>& moves, const Board& board, int depth);
    
    // Helper: update killer moves
    static void update_killer_move(Move move, int depth);
    
    // Helper: update history heuristic
    static void update_history(Move move, const Board& board, int depth);
    
    // Helper: check if move is a killer move
    static bool is_killer_move(Move move, int depth);
};

