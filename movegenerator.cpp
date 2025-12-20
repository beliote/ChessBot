#include "movegenerator.h"
#include "board.h"
#include <iostream>

// Définitions des membres statiques
Bitboard MoveGenerator::PAWN_ATTACKS[2][64];
Bitboard MoveGenerator::KNIGHT_ATTACKS[64];
Bitboard MoveGenerator::KING_ATTACKS[64];

// Tables non utilisées mais requises pour la compilation
Bitboard MoveGenerator::BISHOP_MASKS[64];
Bitboard MoveGenerator::ROOK_MASKS[64];
Bitboard MoveGenerator::BISHOP_ATTACKS[64][512];
Bitboard MoveGenerator::ROOK_ATTACKS[64][4096];
Bitboard MoveGenerator::BISHOP_MAGICS[64];
Bitboard MoveGenerator::ROOK_MAGICS[64];
int MoveGenerator::BISHOP_SHIFT[64];
int MoveGenerator::ROOK_SHIFT[64];

// ============================================================================
// ATTACK GENERATORS (CALCUL DIRECT - COMPATIBLE A8=0)
// ============================================================================

Bitboard MoveGenerator::generate_bishop_attacks_occupancy(Square sq, Bitboard occupancy) {
    Bitboard attacks = 0;
    int rank = rank_of(sq);
    int file = file_of(sq);
    
    // 4 directions diagonales
    for (int r = rank + 1, f = file + 1; r < 8 && f < 8; r++, f++) {
        Bitboard target = (1ULL << square_from_coords(r, f));
        attacks |= target; if (occupancy & target) break;
    }
    for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; r++, f--) {
        Bitboard target = (1ULL << square_from_coords(r, f));
        attacks |= target; if (occupancy & target) break;
    }
    for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; r--, f++) {
        Bitboard target = (1ULL << square_from_coords(r, f));
        attacks |= target; if (occupancy & target) break;
    }
    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
        Bitboard target = (1ULL << square_from_coords(r, f));
        attacks |= target; if (occupancy & target) break;
    }
    return attacks;
}

Bitboard MoveGenerator::generate_rook_attacks_occupancy(Square sq, Bitboard occupancy) {
    Bitboard attacks = 0;
    int rank = rank_of(sq);
    int file = file_of(sq);
    
    // 4 directions orthogonales
    for (int r = rank + 1; r < 8; r++) {
        Bitboard target = (1ULL << square_from_coords(r, file));
        attacks |= target; if (occupancy & target) break;
    }
    for (int r = rank - 1; r >= 0; r--) {
        Bitboard target = (1ULL << square_from_coords(r, file));
        attacks |= target; if (occupancy & target) break;
    }
    for (int f = file + 1; f < 8; f++) {
        Bitboard target = (1ULL << square_from_coords(rank, f));
        attacks |= target; if (occupancy & target) break;
    }
    for (int f = file - 1; f >= 0; f--) {
        Bitboard target = (1ULL << square_from_coords(rank, f));
        attacks |= target; if (occupancy & target) break;
    }
    return attacks;
}

// Bouchons
Bitboard MoveGenerator::generate_bishop_attacks_mask(Square sq) { return 0; }
Bitboard MoveGenerator::generate_rook_attacks_mask(Square sq) { return 0; }
Bitboard MoveGenerator::find_magic_number(Square sq, int relevant_bits, bool is_bishop) { return 0; }
void MoveGenerator::init_slider_attacks(bool is_bishop) {}

// ============================================================================
// INITIALISATION (ADAPTÉE AU MAPPING A8=0)
// ============================================================================

void MoveGenerator::init() {
    // Pions : ATTENTION SENS INVERSÉ PAR RAPPORT AU STANDARD
    // Blancs (montent vers rang 0) : rank - 1
    // Noirs (descendent vers rang 7) : rank + 1
    for (Square sq = 0; sq < 64; sq++) {
        int r = rank_of(sq);
        int f = file_of(sq);

        // Attaques BLANCHES (vers le haut/Rank decreissant)
        PAWN_ATTACKS[WHITE][sq] = 0;
        if (r > 0) { // Si pas sur le bord haut
            if (f > 0) PAWN_ATTACKS[WHITE][sq] |= (1ULL << square_from_coords(r - 1, f - 1));
            if (f < 7) PAWN_ATTACKS[WHITE][sq] |= (1ULL << square_from_coords(r - 1, f + 1));
        }

        // Attaques NOIRES (vers le bas/Rank croissant)
        PAWN_ATTACKS[BLACK][sq] = 0;
        if (r < 7) { // Si pas sur le bord bas
            if (f > 0) PAWN_ATTACKS[BLACK][sq] |= (1ULL << square_from_coords(r + 1, f - 1));
            if (f < 7) PAWN_ATTACKS[BLACK][sq] |= (1ULL << square_from_coords(r + 1, f + 1));
        }
    }
    
    // Cavaliers (inchangé, symétrique)
    const int knight_deltas[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
    for (Square sq = 0; sq < 64; sq++) {
        KNIGHT_ATTACKS[sq] = 0;
        int rank = rank_of(sq);
        int file = file_of(sq);
        for (int i = 0; i < 8; i++) {
            int new_rank = rank + knight_deltas[i][0];
            int new_file = file + knight_deltas[i][1];
            if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8) {
                KNIGHT_ATTACKS[sq] |= (1ULL << square_from_coords(new_rank, new_file));
            }
        }
    }
    
    // Rois (inchangé)
    const int king_deltas[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    for (Square sq = 0; sq < 64; sq++) {
        KING_ATTACKS[sq] = 0;
        int rank = rank_of(sq);
        int file = file_of(sq);
        for (int i = 0; i < 8; i++) {
            int new_rank = rank + king_deltas[i][0];
            int new_file = file + king_deltas[i][1];
            if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8) {
                KING_ATTACKS[sq] |= (1ULL << square_from_coords(new_rank, new_file));
            }
        }
    }
}

// ============================================================================
// RUNTIME GETTERS
// ============================================================================

Bitboard MoveGenerator::get_pawn_attacks(Square sq, Color color) { return PAWN_ATTACKS[color][sq]; }
Bitboard MoveGenerator::get_knight_attacks(Square sq) { return KNIGHT_ATTACKS[sq]; }
Bitboard MoveGenerator::get_king_attacks(Square sq) { return KING_ATTACKS[sq]; }

Bitboard MoveGenerator::get_bishop_attacks(Square sq, Bitboard occupancy) {
    return generate_bishop_attacks_occupancy(sq, occupancy);
}
Bitboard MoveGenerator::get_rook_attacks(Square sq, Bitboard occupancy) {
    return generate_rook_attacks_occupancy(sq, occupancy);
}
Bitboard MoveGenerator::get_queen_attacks(Square sq, Bitboard occupancy) {
    return get_bishop_attacks(sq, occupancy) | get_rook_attacks(sq, occupancy);
}

// ============================================================================
// MOVE GENERATION (ADAPTÉE AU MAPPING A8=0)
// ============================================================================

void MoveGenerator::generate_pawn_moves(const Board& board, std::vector<Move>& moves) {
    Color side = board.side_to_move;
    Bitboard pawns = board.pieces[WHITE_PAWN + side * 6];
    Bitboard all_pieces = board.occupancy[BOTH];
    Bitboard enemy_pieces = board.occupancy[1 - side];
    
    Bitboard pawns_copy = pawns;
    while (pawns_copy) {
        Square from = Bitboards::pop_lsb(pawns_copy);
        int rank = rank_of(from);
        int file = file_of(from);
        
        if (side == WHITE) {
            // DANS VOTRE SYSTÈME (A8=0), LES BLANCS SONT EN BAS (Rang 6/7) ET MONTENT (Vers Rang 0)
            // Donc on fait rank - 1 pour avancer
            
            // Single push
            Square to = square_from_coords(rank - 1, file);
            if (rank > 0 && !Bitboards::get_bit(all_pieces, to)) { // rank > 0 car on monte vers 0
                if (rank == 1) { // Promotion au rang 1 (car rang 0 est la fin)
                    for (int promo = KNIGHT; promo <= QUEEN; promo++) moves.push_back(make_move(from, to, MOVE_TYPE_PROMOTION, promo - KNIGHT));
                } else {
                    moves.push_back(make_move(from, to));
                }
                
                // Double push (Depuis le rang 6 vers le 4)
                if (rank == 6) {
                    Square to2 = square_from_coords(rank - 2, file);
                    if (!Bitboards::get_bit(all_pieces, to2)) moves.push_back(make_move(from, to2));
                }
            }
            
            // Captures (utilise les tables précalculées qui sont maintenant correctes)
            Bitboard attacks = PAWN_ATTACKS[WHITE][from] & enemy_pieces;
            while (attacks) {
                Square to = Bitboards::pop_lsb(attacks);
                if (rank == 1) {
                    for (int promo = KNIGHT; promo <= QUEEN; promo++) moves.push_back(make_move(from, to, MOVE_TYPE_PROMOTION, promo - KNIGHT));
                } else {
                    moves.push_back(make_move(from, to));
                }
            }
            
            // En Passant
            if (board.en_passant_square != NO_SQ) {
               Bitboard ep_attack = PAWN_ATTACKS[WHITE][from] & (1ULL << board.en_passant_square);
               if(ep_attack) moves.push_back(make_move(from, board.en_passant_square, MOVE_TYPE_EN_PASSANT));
            }

        } else { // NOIRS
            // DANS VOTRE SYSTÈME, LES NOIRS SONT EN HAUT (Rang 0/1) ET DESCENDENT (Vers Rang 7)
            // Donc on fait rank + 1 pour avancer
            
            // Single push
            Square to = square_from_coords(rank + 1, file);
            if (rank < 7 && !Bitboards::get_bit(all_pieces, to)) {
                if (rank == 6) { // Promotion au rang 6 (car rang 7 est la fin)
                    for (int promo = KNIGHT; promo <= QUEEN; promo++) moves.push_back(make_move(from, to, MOVE_TYPE_PROMOTION, promo - KNIGHT));
                } else {
                    moves.push_back(make_move(from, to));
                }
                
                // Double push (Depuis le rang 1 vers le 3)
                if (rank == 1) {
                    Square to2 = square_from_coords(rank + 2, file);
                    if (!Bitboards::get_bit(all_pieces, to2)) moves.push_back(make_move(from, to2));
                }
            }
            
            Bitboard attacks = PAWN_ATTACKS[BLACK][from] & enemy_pieces;
            while (attacks) {
                Square to = Bitboards::pop_lsb(attacks);
                if (rank == 6) {
                    for (int promo = KNIGHT; promo <= QUEEN; promo++) moves.push_back(make_move(from, to, MOVE_TYPE_PROMOTION, promo - KNIGHT));
                } else {
                    moves.push_back(make_move(from, to));
                }
            }
            
            if (board.en_passant_square != NO_SQ) {
               Bitboard ep_attack = PAWN_ATTACKS[BLACK][from] & (1ULL << board.en_passant_square);
               if(ep_attack) moves.push_back(make_move(from, board.en_passant_square, MOVE_TYPE_EN_PASSANT));
            }
        }
    }
}

void MoveGenerator::generate_knight_moves(const Board& board, std::vector<Move>& moves) {
    Color side = board.side_to_move;
    Bitboard knights = board.pieces[WHITE_KNIGHT + side * 6];
    Bitboard friendly_pieces = board.occupancy[side];
    while (knights) {
        Square from = Bitboards::pop_lsb(knights);
        Bitboard attacks = KNIGHT_ATTACKS[from] & ~friendly_pieces;
        while (attacks) {
            Square to = Bitboards::pop_lsb(attacks);
            moves.push_back(make_move(from, to));
        }
    }
}

void MoveGenerator::generate_bishop_moves(const Board& board, std::vector<Move>& moves) {
    Color side = board.side_to_move;
    Bitboard bishops = board.pieces[WHITE_BISHOP + side * 6];
    Bitboard friendly_pieces = board.occupancy[side];
    while (bishops) {
        Square from = Bitboards::pop_lsb(bishops);
        Bitboard attacks = get_bishop_attacks(from, board.occupancy[BOTH]) & ~friendly_pieces;
        while (attacks) {
            Square to = Bitboards::pop_lsb(attacks);
            moves.push_back(make_move(from, to));
        }
    }
}

void MoveGenerator::generate_rook_moves(const Board& board, std::vector<Move>& moves) {
    Color side = board.side_to_move;
    Bitboard rooks = board.pieces[WHITE_ROOK + side * 6];
    Bitboard friendly_pieces = board.occupancy[side];
    while (rooks) {
        Square from = Bitboards::pop_lsb(rooks);
        Bitboard attacks = get_rook_attacks(from, board.occupancy[BOTH]) & ~friendly_pieces;
        while (attacks) {
            Square to = Bitboards::pop_lsb(attacks);
            moves.push_back(make_move(from, to));
        }
    }
}

void MoveGenerator::generate_queen_moves(const Board& board, std::vector<Move>& moves) {
    Color side = board.side_to_move;
    Bitboard queens = board.pieces[WHITE_QUEEN + side * 6];
    Bitboard friendly_pieces = board.occupancy[side];
    while (queens) {
        Square from = Bitboards::pop_lsb(queens);
        Bitboard attacks = get_queen_attacks(from, board.occupancy[BOTH]) & ~friendly_pieces;
        while (attacks) {
            Square to = Bitboards::pop_lsb(attacks);
            moves.push_back(make_move(from, to));
        }
    }
}

void MoveGenerator::generate_king_moves(const Board& board, std::vector<Move>& moves) {
    Color side = board.side_to_move;
    Bitboard kings = board.pieces[WHITE_KING + side * 6];
    Bitboard friendly_pieces = board.occupancy[side];
    
    if (kings == 0) return;
    
    Square from = Bitboards::get_lsb_index(kings);
    Bitboard attacks = KING_ATTACKS[from] & ~friendly_pieces;
    while (attacks) {
        Square to = Bitboards::pop_lsb(attacks);
        moves.push_back(make_move(from, to));
    }
    
    // Castling Logic
    // DANS VOTRE SYSTÈME:
    // White King starts at E1 = 60. Castling K -> G1 (62), Q -> C1 (58).
    // Black King starts at E8 = 4.  Castling K -> G8 (6),  Q -> C8 (2).
    
    if (side == WHITE) {
        if ((board.castling_rights & WK) && 
            !Bitboards::get_bit(board.occupancy[BOTH], SQ_F1) && 
            !Bitboards::get_bit(board.occupancy[BOTH], SQ_G1) && from == SQ_E1) {
            if (!board.is_square_attacked(SQ_E1, BLACK) && 
                !board.is_square_attacked(SQ_F1, BLACK) && 
                !board.is_square_attacked(SQ_G1, BLACK)) {
                moves.push_back(make_move(SQ_E1, SQ_G1, MOVE_TYPE_CASTLING));
            }
        }
        if ((board.castling_rights & WQ) && 
            !Bitboards::get_bit(board.occupancy[BOTH], SQ_D1) && 
            !Bitboards::get_bit(board.occupancy[BOTH], SQ_C1) && 
            !Bitboards::get_bit(board.occupancy[BOTH], SQ_B1) && from == SQ_E1) {
            if (!board.is_square_attacked(SQ_E1, BLACK) && 
                !board.is_square_attacked(SQ_D1, BLACK) && 
                !board.is_square_attacked(SQ_C1, BLACK)) {
                moves.push_back(make_move(SQ_E1, SQ_C1, MOVE_TYPE_CASTLING));
            }
        }
    } else {
        if ((board.castling_rights & BK) && 
            !Bitboards::get_bit(board.occupancy[BOTH], SQ_F8) && 
            !Bitboards::get_bit(board.occupancy[BOTH], SQ_G8) && from == SQ_E8) {
            if (!board.is_square_attacked(SQ_E8, WHITE) && 
                !board.is_square_attacked(SQ_F8, WHITE) && 
                !board.is_square_attacked(SQ_G8, WHITE)) {
                moves.push_back(make_move(SQ_E8, SQ_G8, MOVE_TYPE_CASTLING));
            }
        }
        if ((board.castling_rights & BQ) && 
            !Bitboards::get_bit(board.occupancy[BOTH], SQ_D8) && 
            !Bitboards::get_bit(board.occupancy[BOTH], SQ_C8) && 
            !Bitboards::get_bit(board.occupancy[BOTH], SQ_B8) && from == SQ_E8) {
            if (!board.is_square_attacked(SQ_E8, WHITE) && 
                !board.is_square_attacked(SQ_D8, WHITE) && 
                !board.is_square_attacked(SQ_C8, WHITE)) {
                moves.push_back(make_move(SQ_E8, SQ_C8, MOVE_TYPE_CASTLING));
            }
        }
    }
}

void MoveGenerator::generate_moves(const Board& board, std::vector<Move>& moves) {
    moves.clear();
    generate_pawn_moves(board, moves);
    generate_knight_moves(board, moves);
    generate_bishop_moves(board, moves);
    generate_rook_moves(board, moves);
    generate_queen_moves(board, moves);
    generate_king_moves(board, moves);
}

void MoveGenerator::generate_legal_moves(const Board& board, std::vector<Move>& moves) {
    moves.clear();
    std::vector<Move> pseudo_legal_moves;
    generate_moves(board, pseudo_legal_moves);
    
    for (Move move : pseudo_legal_moves) {
        // 1. CRITIQUE : Créer une copie propre du plateau À L'INTÉRIEUR de la boucle
        Board test_board = board; 
        
        // 2. Jouer le coup sur la copie (le trait passe à l'adversaire)
        test_board.make_move(move);
        
        // 3. CRITIQUE : Vérifier si le Roi de celui QUI VIENT DE JOUER est attaqué
        // make_move a inversé side_to_move, donc le joueur qui a fait le coup est (1 - test_board.side_to_move)
        Color mover = (Color)(1 - test_board.side_to_move);
        
        // Trouver le roi du joueur
        int king_idx = (mover == WHITE) ? WHITE_KING : BLACK_KING;
        Bitboard king_bb = test_board.pieces[king_idx];
        
        // S'il y a un roi (sécurité), vérifier s'il est attaqué par le camp adverse (qui a maintenant le trait)
        if (king_bb) {
            Square king_sq = Bitboards::get_lsb_index(king_bb);
            // Est-ce que 'king_sq' est attaqué par 'test_board.side_to_move' ?
            if (!test_board.is_square_attacked(king_sq, test_board.side_to_move)) {
                moves.push_back(move);
            }
        }
    }
}