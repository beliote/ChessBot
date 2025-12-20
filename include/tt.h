#pragma once

#include "types.h"
#include <cstdint>

// Transposition Table entry flags
enum TTFlag {
    TT_EXACT = 0,      // Exact score (alpha == beta)
    TT_ALPHA = 1,      // Upper bound (score <= alpha)
    TT_BETA = 2       // Lower bound (score >= beta)
};

// Transposition Table entry
struct TTEntry {
    uint64_t key;      // Zobrist hash key (to verify it's the same position)
    int score;         // Evaluation score
    Move move;         // Best move found (for move ordering)
    int depth;         // Search depth
    int flag;          // TT_EXACT, TT_ALPHA, or TT_BETA
};

class TT {
public:
    // Initialize transposition table with given size in MB
    static void init(int size_mb);
    
    // Clear the transposition table
    static void clear();
    
    // Probe the transposition table
    // Returns true if entry found and usable, false otherwise
    // If found, fills in score, move, depth, and flag
    static bool probe(uint64_t key, int& score, Move& move, int& depth, int& flag, int current_depth, int alpha, int beta);
    
    // Store entry in transposition table
    // Adjusts mate scores by ply for correct storage
    static void store(uint64_t key, int score, Move move, int depth, int flag, int ply);
    
    // Get hash full percentage (for UCI info)
    static int get_hash_full();
    
private:
    static TTEntry* table;
    static uint64_t size;          // Number of entries
    static uint64_t mask;          // Bitmask for indexing (size - 1)
    static uint64_t entries_written; // For hash full calculation
    
    // Helper: adjust mate scores for storage
    static int adjust_mate_score(int score, int ply);
    
    // Helper: restore mate scores after retrieval
    static int restore_mate_score(int score, int ply);
};

