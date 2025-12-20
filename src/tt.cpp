#include "tt.h"
#include <cstring>
#include <algorithm>
#ifdef _MSC_VER
    #include <intrin.h>
#endif

// Transposition Table
TTEntry* TT::table = nullptr;
uint64_t TT::size = 0;
uint64_t TT::mask = 0;
uint64_t TT::entries_written = 0;

void TT::init(int size_mb) {
    // Calculate number of entries (each entry is ~16 bytes: key=8, score=4, move=2, depth=1, flag=1)
    // Actually, let's use 16 bytes per entry for alignment
    uint64_t bytes = static_cast<uint64_t>(size_mb) * 1024 * 1024;
    uint64_t num_entries = bytes / sizeof(TTEntry);
    
    // Round down to power of 2 for fast indexing
    // Find the largest power of 2 <= num_entries
    #ifdef _MSC_VER
        #include <intrin.h>
        unsigned long index;
        if (_BitScanReverse64(&index, num_entries)) {
            size = 1ULL << index;
        } else {
            size = 1;
        }
    #else
        if (num_entries > 0) {
            size = 1ULL << (63 - __builtin_clzll(num_entries));
        } else {
            size = 1;
        }
    #endif
    
    mask = size - 1;
    
    // Allocate table
    table = new TTEntry[size];
    clear();
}

void TT::clear() {
    if (table) {
        std::memset(table, 0, size * sizeof(TTEntry));
    }
    entries_written = 0;
}

int TT::adjust_mate_score(int score, int ply) {
    // Mate scores are relative to the root
    // When storing, we need to adjust them to be relative to current position
    if (score > 20000) {
        // Mate in N for current side
        return score + ply;
    } else if (score < -20000) {
        // Mate in N for opponent
        return score - ply;
    }
    return score;
}

int TT::restore_mate_score(int score, int ply) {
    // Restore mate scores to be relative to root (they were stored relative to root)
    // Actually, we don't need to adjust here - we'll adjust in the search function
    return score;
}

bool TT::probe(uint64_t key, int& score, Move& move, int& depth, int& flag, 
               int current_depth, int alpha, int beta) {
    if (!table) return false;
    
    uint64_t index = key & mask;
    TTEntry& entry = table[index];
    
    // Check if entry matches (key verification)
    if (entry.key != key) {
        return false;
    }
    
    // Check if entry depth is sufficient
    if (entry.depth < current_depth) {
        return false;
    }
    
    // Restore mate scores (they were stored relative to root, now restore to current position)
    int stored_score = entry.score;
    score = restore_mate_score(stored_score, 0);  // Restore to root-relative
    
    // Return stored values
    move = entry.move;
    depth = entry.depth;
    flag = entry.flag;
    
    // Check if we can use this entry based on flag
    if (entry.flag == TT_EXACT) {
        return true;  // Always usable
    } else if (entry.flag == TT_ALPHA) {
        // Upper bound: stored score <= alpha means we can cutoff
        if (score <= alpha) {
            return true;
        }
    } else if (entry.flag == TT_BETA) {
        // Lower bound: stored score >= beta means we can cutoff
        if (score >= beta) {
            return true;
        }
    }
    
    return false;
}

void TT::store(uint64_t key, int score, Move move, int depth, int flag, int ply) {
    if (!table) return;
    
    uint64_t index = key & mask;
    TTEntry& entry = table[index];
    
    // Adjust mate scores for storage
    score = adjust_mate_score(score, ply);
    
    // Replacement strategy: Always Replace (simple and effective)
    // Alternative: Depth-Preferred (store if new depth >= old depth)
    // For now, use Always Replace for simplicity
    if (entry.key == 0 || entry.depth <= depth) {
        entry.key = key;
        entry.score = score;
        entry.move = move;
        entry.depth = depth;
        entry.flag = flag;
        entries_written++;
    }
}

int TT::get_hash_full() {
    if (!table || size == 0) return 0;
    
    // Count non-empty entries (simple approximation)
    // For better accuracy, we'd need to track this, but this is good enough
    uint64_t sample_size = std::min(size, 1000ULL);
    uint64_t filled = 0;
    
    for (uint64_t i = 0; i < sample_size; i++) {
        if (table[i].key != 0) {
            filled++;
        }
    }
    
    return static_cast<int>((filled * 1000) / sample_size);
}

