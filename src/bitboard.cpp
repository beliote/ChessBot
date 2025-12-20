#include "bitboard.h"
#include <iostream>
#include <iomanip>

namespace Bitboards {

    // Rank masks (rank 0 = 8th rank, rank 7 = 1st rank)
    Bitboard RANK_MASKS[8];
    Bitboard FILE_MASKS[8];
    Bitboard DIAGONAL_MASKS[15];
    Bitboard ANTI_DIAGONAL_MASKS[15];

    int get_diagonal_index(Square sq) {
        int rank = rank_of(sq);
        int file = file_of(sq);
        return rank - file + 7;  // Range: 0-14
    }

    int get_anti_diagonal_index(Square sq) {
        int rank = rank_of(sq);
        int file = file_of(sq);
        return rank + file;  // Range: 0-14
    }

    void init() {
        // Initialize rank masks
        for (int rank = 0; rank < 8; rank++) {
            RANK_MASKS[rank] = 0;
            for (int file = 0; file < 8; file++) {
                Square sq = square_from_coords(rank, file);
                RANK_MASKS[rank] = set_bit(RANK_MASKS[rank], sq);
            }
        }

        // Initialize file masks
        for (int file = 0; file < 8; file++) {
            FILE_MASKS[file] = 0;
            for (int rank = 0; rank < 8; rank++) {
                Square sq = square_from_coords(rank, file);
                FILE_MASKS[file] = set_bit(FILE_MASKS[file], sq);
            }
        }

        // Initialize diagonal masks (a1-h8 diagonals)
        for (int diag = 0; diag < 15; diag++) {
            DIAGONAL_MASKS[diag] = 0;
            for (int rank = 0; rank < 8; rank++) {
                for (int file = 0; file < 8; file++) {
                    if (rank - file + 7 == diag) {
                        Square sq = square_from_coords(rank, file);
                        DIAGONAL_MASKS[diag] = set_bit(DIAGONAL_MASKS[diag], sq);
                    }
                }
            }
        }

        // Initialize anti-diagonal masks (a8-h1 diagonals)
        for (int diag = 0; diag < 15; diag++) {
            ANTI_DIAGONAL_MASKS[diag] = 0;
            for (int rank = 0; rank < 8; rank++) {
                for (int file = 0; file < 8; file++) {
                    if (rank + file == diag) {
                        Square sq = square_from_coords(rank, file);
                        ANTI_DIAGONAL_MASKS[diag] = set_bit(ANTI_DIAGONAL_MASKS[diag], sq);
                    }
                }
            }
        }
    }

    void print_bitboard(Bitboard bb) {
        std::cout << "\n";
        for (int rank = 0; rank < 8; rank++) {
            std::cout << 8 - rank << "  ";
            for (int file = 0; file < 8; file++) {
                Square sq = square_from_coords(rank, file);
                if (get_bit(bb, sq)) {
                    std::cout << "1 ";
                } else {
                    std::cout << ". ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "\n   a b c d e f g h\n\n";
        std::cout << "Bitboard: 0x" << std::hex << std::setw(16) << std::setfill('0') << bb << std::dec << "\n\n";
    }

}

