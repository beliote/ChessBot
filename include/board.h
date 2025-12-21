#pragma once

#include "types.h"
#include "bitboard.h"
#include <vector>  
#include <string>

class Board {
public:
    // Bitboards par type de pièce
    Bitboard pieces[12];  // WHITE_PAWN à BLACK_KING
    Bitboard occupancy[3];  // WHITE, BLACK, BOTH

    // État du plateau
    Color side_to_move;
    Square en_passant_square;
    int castling_rights;
    int halfmove_clock;
    int fullmove_number;
    
    // Zobrist hash key
    uint64_t hash_key;

    // Constructeur
    Board();
    
    // Initialisation depuis FEN
    void set_fen(const std::string& fen);
    
    // Récupérer une pièce
    Piece piece_at(Square sq) const;
    
    // Jouer un coup
    void make_move(Move move);
    // Annuler un coup (Non implémenté dans ce moteur qui utilise la copie de plateau)
    void unmake_move(); 
    
    // Vérifications
    bool is_square_attacked(Square sq, Color by_color) const;
    bool is_in_check() const;
    
    // Affichage
    void print() const;
    std::string get_fen() const;
    
    // Accesseur Hash
    uint64_t get_hash() const { return hash_key; }

    // --- DETECTION DE REPETITION ---
    // On utilise un vecteur dynamique pour stocker les anciennes clés
    std::vector<uint64_t> history;
    bool is_repetition() const;

    // --- NULL MOVE PRUNING ---
    void make_null_move();
    void unmake_null_move();
    Square stored_ep_square;

private:
    // Helper functions internes
    void clear_board();
    void update_occupancy();
    void add_piece(Piece piece, Square sq);
    void remove_piece(Square sq);

    // J'ai supprimé ici l'ancienne structure MoveHistory qui créait le conflit
    // et qui ne servait pas puisque unmake_move est vide.
};