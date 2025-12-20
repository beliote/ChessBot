#pragma once

#include "types.h"
#include "bitboard.h"
#include <string>

class Board {
public:
    // Bitboards for each piece type
    Bitboard pieces[12];  // WHITE_PAWN through BLACK_KING
    Bitboard occupancy[3];  // WHITE, BLACK, BOTH

    // Board state
    Color side_to_move;
    Square en_passant_square;
    int castling_rights;
    int halfmove_clock;
    int fullmove_number;
    
    // Zobrist hash key
    uint64_t hash_key;

    // Constructor
    Board();
    
    // Initialize from FEN string
    void set_fen(const std::string& fen);
    
    // Get piece at square
    Piece piece_at(Square sq) const;
    
    // Make/unmake move
    void make_move(Move move);
    void unmake_move();
    
    // Check if square is attacked by color
    bool is_square_attacked(Square sq, Color by_color) const;
    
    // Check if current side is in check
    bool is_in_check() const;
    
    // Print board
    void print() const;
    
    // Get FEN string
    std::string get_fen() const;
    
    // Get Zobrist hash key
    uint64_t get_hash() const { return hash_key; }

private:
    // Move history for unmake
    struct MoveHistory {
        Move move;
        Square en_passant_square;
        int castling_rights;
        int halfmove_clock;
        Piece captured_piece;
    };
    
    static constexpr int MAX_GAME_LENGTH = 1024;
    MoveHistory history[MAX_GAME_LENGTH];
    int history_ply;
    
    // Helper functions
    void clear_board();
    void update_occupancy();
    void add_piece(Piece piece, Square sq);
    void remove_piece(Square sq);
};

