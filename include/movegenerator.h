#pragma once

#include "types.h"
#include "bitboard.h"
#include <vector>

class MoveGenerator {
public:
    // Pre-calculated attack tables
    static Bitboard PAWN_ATTACKS[2][64];      // [color][square]
    static Bitboard KNIGHT_ATTACKS[64];
    static Bitboard KING_ATTACKS[64];
    static Bitboard BISHOP_MASKS[64];
    static Bitboard ROOK_MASKS[64];
    
    // Magic bitboard tables for sliding pieces
    static Bitboard BISHOP_ATTACKS[64][512];
    static Bitboard ROOK_ATTACKS[64][4096];
    static Bitboard BISHOP_MAGICS[64];
    static Bitboard ROOK_MAGICS[64];
    static int BISHOP_SHIFT[64];
    static int ROOK_SHIFT[64];
    
    // Initialize all attack tables
    static void init();
    
    // Get attack bitboards
    static Bitboard get_pawn_attacks(Square sq, Color color);
    static Bitboard get_knight_attacks(Square sq);
    static Bitboard get_king_attacks(Square sq);
    static Bitboard get_bishop_attacks(Square sq, Bitboard occupancy);
    static Bitboard get_rook_attacks(Square sq, Bitboard occupancy);
    static Bitboard get_queen_attacks(Square sq, Bitboard occupancy);
    
    // Generate moves
    static void generate_pseudo_moves(const class Board& board, std::vector<Move>& moves);
    static void generate_pawn_moves(const class Board& board, std::vector<Move>& moves);
    static void generate_knight_moves(const class Board& board, std::vector<Move>& moves);
    static void generate_bishop_moves(const class Board& board, std::vector<Move>& moves);
    static void generate_rook_moves(const class Board& board, std::vector<Move>& moves);
    static void generate_queen_moves(const class Board& board, std::vector<Move>& moves);
    static void generate_king_moves(const class Board& board, std::vector<Move>& moves);
    
    // Generate only legal moves
    static void generate_legal_moves(const class Board& board, std::vector<Move>& moves);
    
private:
    // Helper functions for magic bitboards
    static Bitboard generate_bishop_attacks_mask(Square sq);
    static Bitboard generate_rook_attacks_mask(Square sq);
    static Bitboard generate_bishop_attacks_occupancy(Square sq, Bitboard occupancy);
    static Bitboard generate_rook_attacks_occupancy(Square sq, Bitboard occupancy);
    static Bitboard find_magic_number(Square sq, int relevant_bits, bool is_bishop);
    static void init_magic_numbers();
    static void init_slider_attacks(bool is_bishop);
    
    // Move generation helpers
    static void add_move_if_legal(class Board& board, Move move, std::vector<Move>& moves);
};

