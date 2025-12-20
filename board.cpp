#include "board.h"
#include "movegenerator.h"
#include <iostream>
#include <sstream>
#include <cctype>
#include <vector>

// ============================================================================
// CONSTRUCTOR & UTILS
// ============================================================================

Board::Board() {
    clear_board();
}

void Board::clear_board() {
    for (int i = 0; i < 12; i++) pieces[i] = 0;
    for (int i = 0; i < 3; i++) occupancy[i] = 0;
    side_to_move = WHITE;
    en_passant_square = NO_SQ;
    castling_rights = 0;
    halfmove_clock = 0;
    fullmove_number = 1;
}

void Board::update_occupancy() {
    occupancy[WHITE] = pieces[WHITE_PAWN] | pieces[WHITE_KNIGHT] | pieces[WHITE_BISHOP] |
                       pieces[WHITE_ROOK] | pieces[WHITE_QUEEN] | pieces[WHITE_KING];
    occupancy[BLACK] = pieces[BLACK_PAWN] | pieces[BLACK_KNIGHT] | pieces[BLACK_BISHOP] |
                       pieces[BLACK_ROOK] | pieces[BLACK_QUEEN] | pieces[BLACK_KING];
    occupancy[BOTH] = occupancy[WHITE] | occupancy[BLACK];
}

void Board::add_piece(Piece piece, Square sq) {
    pieces[piece] = Bitboards::set_bit(pieces[piece], sq);
    update_occupancy();
}

void Board::remove_piece(Square sq) {
    for (int p = 0; p < 12; p++) {
        if (Bitboards::get_bit(pieces[p], sq)) {
            pieces[p] = Bitboards::pop_bit(pieces[p], sq);
            break;
        }
    }
    update_occupancy();
}

Piece Board::piece_at(Square sq) const {
    for (int p = 0; p < 12; p++) {
        if (Bitboards::get_bit(pieces[p], sq)) return p;
    }
    return NO_PIECE;
}

// ============================================================================
// FEN HANDLING
// ============================================================================

void Board::set_fen(const std::string& fen) {
    clear_board();
    std::istringstream ss(fen);
    std::string placement, turn, castling, en_passant;
    ss >> placement >> turn >> castling >> en_passant;

    int rank = 0;
    int file = 0;

    for (char c : placement) {
        if (c == '/') {
            rank++;
            file = 0;
        } else if (isdigit(c)) {
            file += (c - '0');
        } else {
            Piece piece = NO_PIECE;
            switch(c) {
                case 'P': piece = WHITE_PAWN; break;
                case 'N': piece = WHITE_KNIGHT; break;
                case 'B': piece = WHITE_BISHOP; break;
                case 'R': piece = WHITE_ROOK; break;
                case 'Q': piece = WHITE_QUEEN; break;
                case 'K': piece = WHITE_KING; break;
                case 'p': piece = BLACK_PAWN; break;
                case 'n': piece = BLACK_KNIGHT; break;
                case 'b': piece = BLACK_BISHOP; break;
                case 'r': piece = BLACK_ROOK; break;
                case 'q': piece = BLACK_QUEEN; break;
                case 'k': piece = BLACK_KING; break;
            }
            Square sq = square_from_coords(rank, file);
            pieces[piece] = Bitboards::set_bit(pieces[piece], sq);
            file++;
        }
    }

    side_to_move = (turn == "w") ? WHITE : BLACK;

    castling_rights = 0;
    if (castling != "-") {
        if (castling.find('K') != std::string::npos) castling_rights |= WK;
        if (castling.find('Q') != std::string::npos) castling_rights |= WQ;
        if (castling.find('k') != std::string::npos) castling_rights |= BK;
        if (castling.find('q') != std::string::npos) castling_rights |= BQ;
    }

    if (en_passant != "-") {
        int f = en_passant[0] - 'a';
        int r = 8 - (en_passant[1] - '0'); 
        en_passant_square = square_from_coords(r, f);
    } else {
        en_passant_square = NO_SQ;
    }

    update_occupancy();
}

std::string Board::get_fen() const {
    return ""; // Placeholder
}

void Board::print() const {
    std::cout << "\n";
    for (int rank = 0; rank < 8; rank++) {
        std::cout << (8 - rank) << "  ";
        for (int file = 0; file < 8; file++) {
            Square sq = square_from_coords(rank, file);
            Piece piece = piece_at(sq);
            char c = '.';
            if (piece != NO_PIECE) {
                const char* symbols = "PNBRQKpnbrqk";
                c = symbols[piece];
            }
            std::cout << c << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n   a b c d e f g h\n\n";
}

// ============================================================================
// MOVE EXECUTION
// ============================================================================

void Board::make_move(Move move) {
    Square from = get_from_sq(move);
    Square to = get_to_sq(move);
    int move_type = get_move_type(move);
    int promotion = get_promotion(move);
    
    Piece piece = piece_at(from);
    Piece captured = piece_at(to);
    
    // Move piece
    pieces[piece] = Bitboards::pop_bit(pieces[piece], from);
    pieces[piece] = Bitboards::set_bit(pieces[piece], to);
    
    // Capture
    if (captured != NO_PIECE) {
        pieces[captured] = Bitboards::pop_bit(pieces[captured], to);
    }
    
    // Promotion
    if (move_type == MOVE_TYPE_PROMOTION) {
        pieces[piece] = Bitboards::pop_bit(pieces[piece], to);
        int promo_piece = (side_to_move == WHITE) ? (WHITE_KNIGHT + promotion) : (BLACK_KNIGHT + promotion);
        pieces[promo_piece] = Bitboards::set_bit(pieces[promo_piece], to);
    }
    // En Passant
    else if (move_type == MOVE_TYPE_EN_PASSANT) {
        Square capture_sq = (side_to_move == WHITE) ? (to + 8) : (to - 8); 
        Piece ep_pawn = (side_to_move == WHITE) ? BLACK_PAWN : WHITE_PAWN;
        pieces[ep_pawn] = Bitboards::pop_bit(pieces[ep_pawn], capture_sq);
    }
    // Castling
    else if (move_type == MOVE_TYPE_CASTLING) {
        if (to == SQ_G1) { pieces[WHITE_ROOK] = Bitboards::pop_bit(pieces[WHITE_ROOK], SQ_H1); pieces[WHITE_ROOK] = Bitboards::set_bit(pieces[WHITE_ROOK], SQ_F1); }
        else if (to == SQ_C1) { pieces[WHITE_ROOK] = Bitboards::pop_bit(pieces[WHITE_ROOK], SQ_A1); pieces[WHITE_ROOK] = Bitboards::set_bit(pieces[WHITE_ROOK], SQ_D1); }
        else if (to == SQ_G8) { pieces[BLACK_ROOK] = Bitboards::pop_bit(pieces[BLACK_ROOK], SQ_H8); pieces[BLACK_ROOK] = Bitboards::set_bit(pieces[BLACK_ROOK], SQ_F8); }
        else if (to == SQ_C8) { pieces[BLACK_ROOK] = Bitboards::pop_bit(pieces[BLACK_ROOK], SQ_A8); pieces[BLACK_ROOK] = Bitboards::set_bit(pieces[BLACK_ROOK], SQ_D8); }
    }
    
    // Castling rights update
    if (piece == WHITE_KING) castling_rights &= ~(WK | WQ);
    if (piece == BLACK_KING) castling_rights &= ~(BK | BQ);
    if (from == SQ_H1 || to == SQ_H1) castling_rights &= ~WK;
    if (from == SQ_A1 || to == SQ_A1) castling_rights &= ~WQ;
    if (from == SQ_H8 || to == SQ_H8) castling_rights &= ~BK;
    if (from == SQ_A8 || to == SQ_A8) castling_rights &= ~BQ;

    // En Passant update
    en_passant_square = NO_SQ;
    if (piece == WHITE_PAWN && (int)from - (int)to == 16) en_passant_square = from - 8;
    if (piece == BLACK_PAWN && (int)to - (int)from == 16) en_passant_square = from + 8;
    
    side_to_move = 1 - side_to_move;
    update_occupancy();
}

void Board::unmake_move() { }

// ============================================================================
// CHECK DETECTION - VERSION ROBUSTE (SANS MoveGenerator)
// ============================================================================

bool Board::is_square_attacked(Square sq, Color by_color) const {
    int rank = rank_of(sq);
    int file = file_of(sq);

    // 1. PIONS (Logique inversée pour A8=0)
    if (by_color == WHITE) {
        // Attaqué par Pion Blanc ? (Ils sont en bas, Rank > sq, et attaquent vers le haut)
        if (rank < 7 && file > 0) { if (Bitboards::get_bit(pieces[WHITE_PAWN], square_from_coords(rank + 1, file - 1))) return true; }
        if (rank < 7 && file < 7) { if (Bitboards::get_bit(pieces[WHITE_PAWN], square_from_coords(rank + 1, file + 1))) return true; }
    } else {
        // Attaqué par Pion Noir ? (Ils sont en haut, Rank < sq, et attaquent vers le bas)
        if (rank > 0 && file > 0) { if (Bitboards::get_bit(pieces[BLACK_PAWN], square_from_coords(rank - 1, file - 1))) return true; }
        if (rank > 0 && file < 7) { if (Bitboards::get_bit(pieces[BLACK_PAWN], square_from_coords(rank - 1, file + 1))) return true; }
    }

    // 2. CAVALIERS
    int knight_deltas[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
    Bitboard knights = pieces[by_color == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT];
    for (auto& d : knight_deltas) {
        int r = rank + d[0]; int f = file + d[1];
        if (r >= 0 && r < 8 && f >= 0 && f < 8) {
            if (Bitboards::get_bit(knights, square_from_coords(r, f))) return true;
        }
    }

    // 3. ROIS
    int king_deltas[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    Bitboard king = pieces[by_color == WHITE ? WHITE_KING : BLACK_KING];
    for (auto& d : king_deltas) {
        int r = rank + d[0]; int f = file + d[1];
        if (r >= 0 && r < 8 && f >= 0 && f < 8) {
            if (Bitboards::get_bit(king, square_from_coords(r, f))) return true;
        }
    }

    // 4. DIAGONALES (Fous/Dames) - Scan direct
    Bitboard diag_attackers = pieces[by_color == WHITE ? WHITE_BISHOP : BLACK_BISHOP] | pieces[by_color == WHITE ? WHITE_QUEEN : BLACK_QUEEN];
    int diag_dirs[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    for (auto& d : diag_dirs) {
        for (int dist = 1; dist < 8; dist++) {
            int r = rank + d[0] * dist;
            int f = file + d[1] * dist;
            if (r < 0 || r >= 8 || f < 0 || f >= 8) break;
            Square target = square_from_coords(r, f);
            if (Bitboards::get_bit(occupancy[BOTH], target)) {
                if (Bitboards::get_bit(diag_attackers, target)) return true;
                break; // Bloqué par une autre pièce
            }
        }
    }

    // 5. ORTHOGONALES (Tours/Dames) - Scan direct
    Bitboard ortho_attackers = pieces[by_color == WHITE ? WHITE_ROOK : BLACK_ROOK] | pieces[by_color == WHITE ? WHITE_QUEEN : BLACK_QUEEN];
    int ortho_dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (auto& d : ortho_dirs) {
        for (int dist = 1; dist < 8; dist++) {
            int r = rank + d[0] * dist;
            int f = file + d[1] * dist;
            if (r < 0 || r >= 8 || f < 0 || f >= 8) break;
            Square target = square_from_coords(r, f);
            if (Bitboards::get_bit(occupancy[BOTH], target)) {
                if (Bitboards::get_bit(ortho_attackers, target)) return true;
                break; // Bloqué
            }
        }
    }

    return false;
}

bool Board::is_in_check() const {
    int king_idx = (side_to_move == WHITE) ? WHITE_KING : BLACK_KING;
    Bitboard king_bb = pieces[king_idx];
    if (king_bb == 0) return true;
    Square king_sq = Bitboards::get_lsb_index(king_bb);
    return is_square_attacked(king_sq, (Color)(1 - side_to_move));
}