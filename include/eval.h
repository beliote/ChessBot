#pragma once

#include "board.h"

class Evaluation {
public:
    // Material values
    static constexpr int PAWN_VALUE = 93;
    static constexpr int KNIGHT_VALUE = 320;
    static constexpr int BISHOP_VALUE = 330;
    static constexpr int ROOK_VALUE = 499;
    static constexpr int QUEEN_VALUE = 900;
    static constexpr int KING_VALUE = 20000;
    
    // Evaluate position from side_to_move's perspective
    static int evaluate(const Board& board);
    
    // Get material value of a piece (public for move ordering)
    static int get_piece_value(Piece piece);
    
private:
    // Piece-square tables (adapted for A8=0 system)
    static const int PAWN_PST[64];
    static const int KNIGHT_PST[64];
    static const int BISHOP_PST[64];
    static const int ROOK_PST[64];
    static const int QUEEN_PST[64];
    static const int KING_PST[64];
    
    // Helper functions
    static int get_pst_value(Piece piece, Square sq);
};

