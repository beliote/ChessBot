#include "zobrist.h"
#include <random>
#include <chrono>

namespace Zobrist {
    // Zobrist hash keys
    uint64_t piece_keys[12][64];
    uint64_t en_passant_keys[64];
    uint64_t castle_keys[16];
    uint64_t side_key;
    
    // Simple PRNG for Zobrist keys (using linear congruential generator)
    // This is deterministic and fast
    uint64_t random_u64() {
        static uint64_t seed = std::chrono::steady_clock::now().time_since_epoch().count();
        seed ^= seed >> 12;
        seed ^= seed << 25;
        seed ^= seed >> 27;
        return seed * 0x2545F4914F6CDD1DULL;
    }
    
    void init_zobrist() {
        // Initialize piece keys for all 12 piece types on all 64 squares
        // A8=0 coordinate system: square 0 = A8, square 63 = H1
        for (int piece = 0; piece < 12; piece++) {
            for (int sq = 0; sq < 64; sq++) {
                piece_keys[piece][sq] = random_u64();
            }
        }
        
        // Initialize en passant keys for all 64 squares
        for (int sq = 0; sq < 64; sq++) {
            en_passant_keys[sq] = random_u64();
        }
        
        // Initialize castling keys for all 16 possible combinations
        // Castling rights: WK=1, WQ=2, BK=4, BQ=8 (0-15 total)
        for (int rights = 0; rights < 16; rights++) {
            castle_keys[rights] = random_u64();
        }
        
        // Initialize side to move key
        side_key = random_u64();
    }
}

