#pragma once

#include "types.h"
#include <cstdint>

namespace Zobrist {
    // Zobrist hash keys
    extern uint64_t piece_keys[12][64];      // [piece][square] - A8=0 coordinate system
    extern uint64_t en_passant_keys[64];     // [square] - for en passant squares
    extern uint64_t castle_keys[16];        // [castling_rights] - 4 bits = 16 combinations
    extern uint64_t side_key;                // XOR when black to move
    
    // Initialize all Zobrist keys with random 64-bit numbers
    void init_zobrist();
    
    // Helper: generate a random 64-bit number
    uint64_t random_u64();
}

