#pragma once
#include "board.h"
#include <string>
#include <unordered_map>
#include <vector>

class Book {
public:
    static void init();
    static Move get_book_move(const Board& board);

private:
    // Map : FEN partiel -> Liste de coups possibles (en notation UCI "e2e4")
    static std::unordered_map<std::string, std::vector<std::string>> book_moves;
    
    // Helper pour nettoyer le FEN (garder seulement les pièces et le trait)
    static std::string clean_fen(const std::string& fen);
};