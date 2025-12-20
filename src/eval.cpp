#include "eval.h"
#include "movegenerator.h"
#include "bitboard.h"

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
    // Rank 0 (A8-H8, indices 0-7) - White promotion rank (HIGH positive value)
    900, 900, 900, 900, 900, 900, 900, 900,
    // Rank 1 (A7-H7, indices 8-15) - Part of Black starting rank
    -50, -50, -50, -50, -50, -50, -50, -50,
    // Rank 2 (A6-H6, indices 16-23)
    10, 10, 20, 30, 30, 20, 10, 10,
    // Rank 3 (A5-H5, indices 24-31)
    20, 20, 30, 40, 40, 30, 20, 20,
    // Rank 4 (A4-H4, indices 32-39)
    30, 30, 40, 50, 50, 40, 30, 30,
    // Rank 5 (A3-H3, indices 40-47)
    40, 40, 50, 60, 60, 50, 40, 40,
    // Rank 6 (A2-H2, indices 48-55) - Part of White starting rank
    -50, -50, -50, -50, -50, -50, -50, -50,
    // Rank 7 (A1-H1, indices 56-63) - Black promotion rank (HIGH positive value, used when flipped)
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
    
    // Count pieces for opening detection
    int white_piece_count = 0;
    int black_piece_count = 0;
    
    // Evaluate all pieces
    for (Square sq = 0; sq < 64; sq++) {
        Piece piece = board.piece_at(sq);
        if (piece == NO_PIECE) continue;
        
        int material = get_piece_value(piece);
        int positional = get_pst_value(piece, sq);
        
        if (piece < BLACK_PAWN) {
            // White piece
            white_score += material + positional;
            white_piece_count++;
            
            // Mobility bonus for white pieces (except pawns and king)
            if (piece == WHITE_KNIGHT || piece == WHITE_BISHOP || 
                piece == WHITE_ROOK || piece == WHITE_QUEEN) {
                Bitboard attacks = 0;
                
                switch (piece) {
                    case WHITE_KNIGHT:
                        attacks = MoveGenerator::get_knight_attacks(sq);
                        break;
                    case WHITE_BISHOP:
                        attacks = MoveGenerator::get_bishop_attacks(sq, board.occupancy[BOTH]);
                        break;
                    case WHITE_ROOK:
                        attacks = MoveGenerator::get_rook_attacks(sq, board.occupancy[BOTH]);
                        break;
                    case WHITE_QUEEN:
                        attacks = MoveGenerator::get_queen_attacks(sq, board.occupancy[BOTH]);
                        break;
                }
                
                // Count attacked squares (mobility)
                int mobility = Bitboards::count_bits(attacks & ~board.occupancy[WHITE]);
                white_score += mobility * 3;  // 3 cp per attacked square
            }
            
            // Opening penalties for white
            if (white_piece_count <= 16) {  // Opening phase
                // Penalty for moving king early (unless castling)
                if (piece == WHITE_KING) {
                    int rank = rank_of(sq);
                    // King should stay on back rank (rank 6-7) in opening
                    if (rank < 6) {
                        white_score -= 20;  // Penalty for moving king forward
                    }
                }
            }
        } else {
            // Black piece
            black_score += material + positional;
            black_piece_count++;
            
            // Mobility bonus for black pieces (except pawns and king)
            if (piece == BLACK_KNIGHT || piece == BLACK_BISHOP || 
                piece == BLACK_ROOK || piece == BLACK_QUEEN) {
                Bitboard attacks = 0;
                
                switch (piece) {
                    case BLACK_KNIGHT:
                        attacks = MoveGenerator::get_knight_attacks(sq);
                        break;
                    case BLACK_BISHOP:
                        attacks = MoveGenerator::get_bishop_attacks(sq, board.occupancy[BOTH]);
                        break;
                    case BLACK_ROOK:
                        attacks = MoveGenerator::get_rook_attacks(sq, board.occupancy[BOTH]);
                        break;
                    case BLACK_QUEEN:
                        attacks = MoveGenerator::get_queen_attacks(sq, board.occupancy[BOTH]);
                        break;
                }
                
                // Count attacked squares (mobility)
                int mobility = Bitboards::count_bits(attacks & ~board.occupancy[BLACK]);
                black_score += mobility * 3;  // 3 cp per attacked square
            }
            
            // Opening penalties for black
            if (black_piece_count <= 16) {  // Opening phase
                // Penalty for moving king early (unless castling)
                if (piece == BLACK_KING) {
                    int rank = rank_of(sq);
                    // King should stay on back rank (rank 0-1) in opening
                    if (rank > 1) {
                        black_score -= 20;  // Penalty for moving king forward
                    }
                }
            }
        }
    }
    
    // Opening penalty: blocking center pawns
    // Check if center pawns (d/e files) are blocked by own pieces
    if (white_piece_count <= 16 || black_piece_count <= 16) {
        // White center pawns (d2, e2 in A8=0: rank 6, files 3,4)
        Square d2 = square_from_coords(6, 3);
        Square e2 = square_from_coords(6, 4);
        if (board.piece_at(d2) == WHITE_PAWN) {
            Square d3 = square_from_coords(5, 3);
            if (board.piece_at(d3) != NO_PIECE && board.piece_at(d3) < BLACK_PAWN) {
                white_score -= 15;  // Penalty for blocking own center pawn
            }
        }
        if (board.piece_at(e2) == WHITE_PAWN) {
            Square e3 = square_from_coords(5, 4);
            if (board.piece_at(e3) != NO_PIECE && board.piece_at(e3) < BLACK_PAWN) {
                white_score -= 15;
            }
        }
        
        // Black center pawns (d7, e7 in A8=0: rank 1, files 3,4)
        Square d7 = square_from_coords(1, 3);
        Square e7 = square_from_coords(1, 4);
        if (board.piece_at(d7) == BLACK_PAWN) {
            Square d6 = square_from_coords(2, 3);
            if (board.piece_at(d6) != NO_PIECE && board.piece_at(d6) >= BLACK_PAWN) {
                black_score -= 15;  // Penalty for blocking own center pawn
            }
        }
        if (board.piece_at(e7) == BLACK_PAWN) {
            Square e6 = square_from_coords(2, 4);
            if (board.piece_at(e6) != NO_PIECE && board.piece_at(e6) >= BLACK_PAWN) {
                black_score -= 15;
            }
        }
    }
    
    // Calculate score from white's perspective: (WhiteScore - BlackScore)
    int score = white_score - black_score;
    
    // CRITICAL: Negamax expects score relative to side to move
    // If it's black's turn, negate the score (black wants to minimize white's advantage)
    // If it's white's turn, return as-is (white wants to maximize)
    return (board.side_to_move == WHITE) ? score : -score;
}

