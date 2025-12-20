#include "eval.h"

// Piece-Square Tables adapted for A8=0 coordinate system
// White pieces: start at rank 6-7 (squares 48-63), move UP (decrease index)
// Black pieces: start at rank 0-1 (squares 0-15), move DOWN (increase index)

// Pawn PST: Encourage advancement towards promotion
// White: rank 6 (48-55) = starting, rank 0 (0-7) = promotion (high bonus)
// Black: rank 1 (8-15) = starting, rank 7 (56-63) = promotion (high bonus)
const int Evaluation::PAWN_PST[64] = {
    // Rank 0 (A8-H8) - White promotion rank (high value)
    900, 900, 900, 900, 900, 900, 900, 900,
    // Rank 1 (A7-H7) - Black starting rank
    -50, -50, -50, -50, -50, -50, -50, -50,
    // Rank 2 (A6-H6)
    10, 10, 20, 30, 30, 20, 10, 10,
    // Rank 3 (A5-H5)
    20, 20, 30, 40, 40, 30, 20, 20,
    // Rank 4 (A4-H4)
    30, 30, 40, 50, 50, 40, 30, 30,
    // Rank 5 (A3-H3)
    40, 40, 50, 60, 60, 50, 40, 40,
    // Rank 6 (A2-H2) - White starting rank
    -50, -50, -50, -50, -50, -50, -50, -50,
    // Rank 7 (A1-H1) - Black promotion rank (high value)
    900, 900, 900, 900, 900, 900, 900, 900
};

// Knight PST: Encourage centralization
const int Evaluation::KNIGHT_PST[64] = {
    // Rank 0
    -50, -40, -30, -30, -30, -30, -40, -50,
    // Rank 1
    -40, -20,   0,   0,   0,   0, -20, -40,
    // Rank 2
    -30,   0,  10,  15,  15,  10,   0, -30,
    // Rank 3
    -30,   5,  15,  20,  20,  15,   5, -30,
    // Rank 4
    -30,   0,  15,  20,  20,  15,   0, -30,
    // Rank 5
    -30,   5,  10,  15,  15,  10,   5, -30,
    // Rank 6
    -40, -20,   0,   5,   5,   0, -20, -40,
    // Rank 7
    -50, -40, -30, -30, -30, -30, -40, -50
};

// Bishop PST: Encourage centralization and long diagonals
const int Evaluation::BISHOP_PST[64] = {
    // Rank 0
    -20, -10, -10, -10, -10, -10, -10, -20,
    // Rank 1
    -10,   0,   0,   0,   0,   0,   0, -10,
    // Rank 2
    -10,   0,   5,  10,  10,   5,   0, -10,
    // Rank 3
    -10,   5,   5,  10,  10,   5,   5, -10,
    // Rank 4
    -10,   0,  10,  10,  10,  10,   0, -10,
    // Rank 5
    -10,  10,  10,  10,  10,  10,  10, -10,
    // Rank 6
    -10,   5,   0,   0,   0,   0,   5, -10,
    // Rank 7
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
    int white_score = 0;
    int black_score = 0;
    
    // Evaluate all pieces
    for (Square sq = 0; sq < 64; sq++) {
        Piece piece = board.piece_at(sq);
        if (piece == NO_PIECE) continue;
        
        int material = get_piece_value(piece);
        int positional = get_pst_value(piece, sq);
        
        if (piece < BLACK_PAWN) {
            // White piece
            white_score += material + positional;
        } else {
            // Black piece
            black_score += material + positional;
        }
    }
    
    // Calculate score from white's perspective
    int score = white_score - black_score;
    
    // Return score relative to side to move
    if (board.side_to_move == BLACK) {
        return -score;
    }
    return score;
}

