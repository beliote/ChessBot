#pragma once

#include "types.h"
#include <iostream>

#ifdef _MSC_VER
    #include <intrin.h>
    #pragma intrinsic(_BitScanForward64)
#endif

namespace Bitboards {

    // Set bit (Allume un bit)
    inline Bitboard set_bit(Bitboard bb, Square sq) {
        return bb | (1ULL << sq);
    }

    // Get bit (Vérifie un bit)
    inline bool get_bit(Bitboard bb, Square sq) {
        return (bb & (1ULL << sq));
    }

    // Pop bit (Retire un bit spécifique) - C'est celle qui manquait !
    inline Bitboard pop_bit(Bitboard bb, Square sq) {
        return bb & ~(1ULL << sq);
    }

    // Count bits (Compte le nombre de pièces)
    inline int count_bits(Bitboard bb) {
        #if defined(_MSC_VER) && defined(_WIN64)
            return (int)__popcnt64(bb);
        #elif defined(__GNUC__) || defined(__clang__)
            return __builtin_popcountll(bb);
        #else
            int count = 0;
            while (bb) {
                count++;
                bb &= bb - 1;
            }
            return count;
        #endif
    }

    // Get Least Significant Bit Index (Trouve l'index du premier bit à 1)
    inline int get_lsb_index(Bitboard bb) {
        if (bb == 0) return -1;

        #if defined(_MSC_VER) && defined(_WIN64)
            unsigned long index;
            _BitScanForward64(&index, bb);
            return (int)index;
        #elif defined(__GNUC__) || defined(__clang__)
            return __builtin_ctzll(bb);
        #else
            return count_bits((bb & -bb) - 1);
        #endif
    }

    // Pop LSB (Retire le premier bit à 1 et retourne son index)
    // Passage par référence (Bitboard& bb) pour modifier la variable originale
    inline Square pop_lsb(Bitboard& bb) {
        Square sq = get_lsb_index(bb);
        bb &= (bb - 1); 
        return sq;
    }

    // Variables globales
    extern Bitboard RANK_MASKS[8];
    extern Bitboard FILE_MASKS[8];
    extern Bitboard DIAGONAL_MASKS[15];
    extern Bitboard ANTI_DIAGONAL_MASKS[15];

    void init();
    void print_bitboard(Bitboard bb);
}