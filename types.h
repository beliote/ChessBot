#pragma once

#include <cstdint>

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

using Bitboard = uint64_t;
using Square = int;
using Piece = int;
using Color = int;

// ============================================================================
// SQUARE CONSTANTS
// ============================================================================

// On garde uniquement les constantes constexpr, on supprime l'enum conflictuel
constexpr Square SQ_A8 = 0;  constexpr Square SQ_B8 = 1;  constexpr Square SQ_C8 = 2;  constexpr Square SQ_D8 = 3;
constexpr Square SQ_E8 = 4;  constexpr Square SQ_F8 = 5;  constexpr Square SQ_G8 = 6;  constexpr Square SQ_H8 = 7;
constexpr Square SQ_A7 = 8;  constexpr Square SQ_B7 = 9;  constexpr Square SQ_C7 = 10; constexpr Square SQ_D7 = 11;
constexpr Square SQ_E7 = 12; constexpr Square SQ_F7 = 13; constexpr Square SQ_G7 = 14; constexpr Square SQ_H7 = 15;
constexpr Square SQ_A6 = 16; constexpr Square SQ_B6 = 17; constexpr Square SQ_C6 = 18; constexpr Square SQ_D6 = 19;
constexpr Square SQ_E6 = 20; constexpr Square SQ_F6 = 21; constexpr Square SQ_G6 = 22; constexpr Square SQ_H6 = 23;
constexpr Square SQ_A5 = 24; constexpr Square SQ_B5 = 25; constexpr Square SQ_C5 = 26; constexpr Square SQ_D5 = 27;
constexpr Square SQ_E5 = 28; constexpr Square SQ_F5 = 29; constexpr Square SQ_G5 = 30; constexpr Square SQ_H5 = 31;
constexpr Square SQ_A4 = 32; constexpr Square SQ_B4 = 33; constexpr Square SQ_C4 = 34; constexpr Square SQ_D4 = 35;
constexpr Square SQ_E4 = 36; constexpr Square SQ_F4 = 37; constexpr Square SQ_G4 = 38; constexpr Square SQ_H4 = 39;
constexpr Square SQ_A3 = 40; constexpr Square SQ_B3 = 41; constexpr Square SQ_C3 = 42; constexpr Square SQ_D3 = 43;
constexpr Square SQ_E3 = 44; constexpr Square SQ_F3 = 45; constexpr Square SQ_G3 = 46; constexpr Square SQ_H3 = 47;
constexpr Square SQ_A2 = 48; constexpr Square SQ_B2 = 49; constexpr Square SQ_C2 = 50; constexpr Square SQ_D2 = 51;
constexpr Square SQ_E2 = 52; constexpr Square SQ_F2 = 53; constexpr Square SQ_G2 = 54; constexpr Square SQ_H2 = 55;
constexpr Square SQ_A1 = 56; constexpr Square SQ_B1 = 57; constexpr Square SQ_C1 = 58; constexpr Square SQ_D1 = 59;
constexpr Square SQ_E1 = 60; constexpr Square SQ_F1 = 61; constexpr Square SQ_G1 = 62; constexpr Square SQ_H1 = 63;
constexpr Square NO_SQ = 64;

// ============================================================================
// PIECE CONSTANTS
// ============================================================================

enum PieceType {
    PAWN = 0, KNIGHT = 1, BISHOP = 2, ROOK = 3, QUEEN = 4, KING = 5
};

enum PieceEnum {
    WHITE_PAWN = 0, WHITE_KNIGHT = 1, WHITE_BISHOP = 2, WHITE_ROOK = 3, WHITE_QUEEN = 4, WHITE_KING = 5,
    BLACK_PAWN = 6, BLACK_KNIGHT = 7, BLACK_BISHOP = 8, BLACK_ROOK = 9, BLACK_QUEEN = 10, BLACK_KING = 11,
    NO_PIECE = 12
};

enum ColorEnum {
    WHITE = 0, BLACK = 1, BOTH = 2
};

enum CastlingRights {
    WK = 1, WQ = 2, BK = 4, BQ = 8
};

// ============================================================================
// MOVE REPRESENTATION
// ============================================================================

using Move = uint16_t;

constexpr Move MOVE_NONE = 0;
constexpr int MOVE_TYPE_NORMAL = 0;
constexpr int MOVE_TYPE_PROMOTION = 1;
constexpr int MOVE_TYPE_EN_PASSANT = 2;
constexpr int MOVE_TYPE_CASTLING = 3;

// Macros
constexpr Move make_move(Square from, Square to, int move_type = MOVE_TYPE_NORMAL, int promotion = 0) {
    return static_cast<Move>((from) | (to << 6) | (promotion << 12) | (move_type << 14));
}

constexpr Square get_from_sq(Move move) { return static_cast<Square>(move & 0x3F); }
constexpr Square get_to_sq(Move move) { return static_cast<Square>((move >> 6) & 0x3F); }
constexpr int get_move_type(Move move) { return (move >> 14) & 0x3; }
constexpr int get_promotion(Move move) { return (move >> 12) & 0x3; }

// Helpers
constexpr Square square_from_coords(int rank, int file) { return static_cast<Square>(rank * 8 + file); }
constexpr int rank_of(Square sq) { return sq / 8; }
constexpr int file_of(Square sq) { return sq % 8; }