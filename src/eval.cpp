#include "eval.h"
#include "movegenerator.h"
#include "bitboard.h"
#include <cctype>

extern int evaluate_nnue(const Board& board);

// Piece-Square Tables adapted for A8=0 coordinate system
// White pieces: start at rank 6-7 (squares 48-63), move UP (decrease index) toward rank 0
// Black pieces: start at rank 0-1 (squares 0-15), move DOWN (increase index) toward rank 7
//
// Coordinate mapping: rank 0 = 8th rank (A8-H8), rank 7 = 1st rank (A1-H1)
// White promotes at rank 0 (indices 0-7, A8-H8) -> HIGH positive values
// Black promotes at rank 7 (indices 56-63, A1-H1) -> HIGH positive values (when flipped)

// Pawn PST: Encourage advancement towards promotion
// Values are from White's perspective. For Black pieces, we flip the table (63 - sq).
// White: rank 6-7 (48-63) = starting, rank 0 (0-7) = promotion (high bonus)
// Black: rank 0-1 (0-15) = starting, rank 7 (56-63) = promotion (high bonus when flipped)
const int Evaluation::PAWN_PST[64] = {
    // Rank 0 (Promotion)
    0,   0,   0,   0,   0,   0,   0,   0,
    // Rank 1
    50, 50, 50, 10, 10, 50, 50, 50,
    // Rank 2 (6ème rangée relative: très avancée)
    10, 10, 20, 30, 30, 20, 10, 10,
    // Rank 3 (5ème rangée relative: e5, d5...)
     5,  5, 10, 25, 25, 10,  5,  5,
    // Rank 4 (4ème rangée relative: e4, d4...) - LE CENTRE EST ROI
     0,  0,  0, 20, 20,  0,  0,  0,
    // Rank 5 (3ème rangée relative: e3, d3...) - On punit a3/h3 !
     5, -5, -10,  0,  0, -10, -5,  5,
    // Rank 6 (Départ Blancs)
    50, 50, 50, 10, 10, 50, 50, 50,
    // Rank 7 (Départ Noirs - utilisé inversé)
     0,   0,   0,   0,   0,   0,   0,   0
};

const int Evaluation::KNIGHT_PST[64] = {
    // Rank 0
    -50, -40, -30, -30, -30, -30, -40, -50,
    // Rank 1
    -40, -20,   0,   0,   0,   0, -20, -40,
    // Rank 2
    -30,   0,  10,  15,  15,  10,   0, -30,
    // Rank 3 (Centre fort)
    -30,   5,  15,  20,  20,  15,   5, -30,
    // Rank 4 (Centre fort)
    -30,   0,  15,  20,  20,  15,   0, -30,
    // Rank 5
    -30,   5,  10,  15,  15,  10,   5, -30,
    // Rank 6 (On décourage les cavaliers de rester derrière)
    -40, -20,   0,   5,   5,   0, -20, -40,
    // Rank 7
    -50, -40, -30, -30, -30, -30, -40, -50
};

const int Evaluation::BISHOP_PST[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,  10,  10,   5,   0, -10,
    -10,   5,   5,  10,  10,   5,   5, -10,
    -10,   0,  10,  10,  10,  10,   0, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

// Rook PST: Encourage central files and 7th/2nd ranks
const int Evaluation::ROOK_PST[64] = {
    // Rank 0 (7th rank for white)
    0,  0,  0,  5,  5,  0,  0,  0,
    // Rank 1
    -5,  0,  0,  0,  0,  0,  0, -5,
    // Rank 2
    -5,  0,  0,  0,  0,  0,  0, -5,
    // Rank 3
    -5,  0,  0,  0,  0,  0,  0, -5,
    // Rank 4
    -5,  0,  0,  0,  0,  0,  0, -5,
    // Rank 5
    -5,  0,  0,  0,  0,  0,  0, -5,
    // Rank 6
    5, 10, 10, 10, 10, 10, 10,  5,
    // Rank 7 (2nd rank for black)
    0,  0,  0,  5,  5,  0,  0,  0
};

// Queen PST: Encourage centralization
const int Evaluation::QUEEN_PST[64] = {
    // Rank 0
    -20, -10, -10,  -5,  -5, -10, -10, -20,
    // Rank 1
    -10,   0,   0,   0,   0,   0,   0, -10,
    // Rank 2
    -10,   0,   5,   5,   5,   5,   0, -10,
    // Rank 3
     -5,   0,   5,   5,   5,   5,   0,  -5,
    // Rank 4
     -5,   0,   5,   5,   5,   5,   0,  -5,
    // Rank 5
    -10,   5,   5,   5,   5,   5,   5, -10,
    // Rank 6
    -10,   0,   5,   0,   0,   0,   0, -10,
    // Rank 7
    -20, -10, -10,  -5,  -5, -10, -10, -20
};

// King PST: Encourage safety in endgame, centralization in opening
const int Evaluation::KING_PST[64] = {
    // Rank 0
    -30, -40, -40, -50, -50, -40, -40, -30,
    // Rank 1
    -30, -40, -40, -50, -50, -40, -40, -30,
    // Rank 2
    -30, -40, -40, -50, -50, -40, -40, -30,
    // Rank 3
    -30, -40, -40, -50, -50, -40, -40, -30,
    // Rank 4
    -20, -30, -30, -40, -40, -30, -30, -20,
    // Rank 5
    -10, -20, -20, -20, -20, -20, -20, -10,
    // Rank 6
     20,  20,   0,   0,   0,   0,  20,  20,
    // Rank 7
     20,  30,  10,   0,   0,  10,  30,  20
};

int Evaluation::get_piece_value(Piece piece) {
    switch (piece) {
        case WHITE_PAWN:   case BLACK_PAWN:   return PAWN_VALUE;
        case WHITE_KNIGHT: case BLACK_KNIGHT: return KNIGHT_VALUE;
        case WHITE_BISHOP: case BLACK_BISHOP: return BISHOP_VALUE;
        case WHITE_ROOK:   case BLACK_ROOK:    return ROOK_VALUE;
        case WHITE_QUEEN:  case BLACK_QUEEN:   return QUEEN_VALUE;
        case WHITE_KING:   case BLACK_KING:    return KING_VALUE;
        default: return 0;
    }
}

int Evaluation::get_pst_value(Piece piece, Square sq) {
    int pst_index = sq;
    
    // For black pieces, flip the PST (mirror vertically)
    if (piece >= BLACK_PAWN) {
        pst_index = 63 - sq;  // Flip rank
    }
    
    switch (piece) {
        case WHITE_PAWN:   case BLACK_PAWN:   return PAWN_PST[pst_index];
        case WHITE_KNIGHT: case BLACK_KNIGHT: return KNIGHT_PST[pst_index];
        case WHITE_BISHOP: case BLACK_BISHOP: return BISHOP_PST[pst_index];
        case WHITE_ROOK:   case BLACK_ROOK:    return ROOK_PST[pst_index];
        case WHITE_QUEEN:  case BLACK_QUEEN:   return QUEEN_PST[pst_index];
        case WHITE_KING:   case BLACK_KING:    return KING_PST[pst_index];
        default: return 0;
    }
}

int Evaluation::evaluate(const Board& board) {
    return evaluate_nnue(board);
}

