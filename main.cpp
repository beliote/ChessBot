#include "board.h"
#include "movegenerator.h"
#include "bitboard.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>

// Perft robuste par copie (évite les bugs de unmake_move)
uint64_t perft(Board board, int depth) { // Note: passage par valeur ou copie interne
    if (depth == 0) return 1;

    std::vector<Move> moves;
    MoveGenerator::generate_legal_moves(board, moves);

    if (depth == 1) return moves.size();

    uint64_t nodes = 0;
    for (const auto& move : moves) {
        Board next_board = board; // COPIE L'ETAT
        next_board.make_move(move);
        nodes += perft(next_board, depth - 1);
    }
    return nodes;
}

// Perft détaillé pour le debug
void run_perft_suite(int depth) {
    std::cout << "\n==========================================\n";
    std::cout << "  PERFORMANCE TEST (PERFT) - DEPTH " << depth << "\n";
    std::cout << "==========================================\n";

    Bitboards::init();
    MoveGenerator::init();
    
    Board board;
    board.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<Move> moves;
    MoveGenerator::generate_legal_moves(board, moves);
    
    uint64_t total_nodes = 0;
    
    for (const auto& move : moves) {
        Board next_board = board;
        next_board.make_move(move);
        
        uint64_t nodes = perft(next_board, depth - 1);
        
        Square from = get_from_sq(move);
        Square to = get_to_sq(move);
        
        // Affichage A8=0 compatible (Conversion index -> notation)
        // Rank 0 = '8', Rank 7 = '1'
        char f1 = 'a' + file_of(from);
        char r1 = '8' - rank_of(from); 
        char f2 = 'a' + file_of(to);
        char r2 = '8' - rank_of(to);
        
        std::cout << f1 << r1 << f2 << r2 << ": " << nodes << "\n";
        total_nodes += nodes;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    
    std::cout << "\n------------------------------------------\n";
    std::cout << "Total Nodes: " << total_nodes << "\n";
    std::cout << "Time:        " << std::fixed << std::setprecision(3) << elapsed.count() << " s\n";
    std::cout << "NPS:         " << (uint64_t)(total_nodes / elapsed.count()) << "\n";
    std::cout << "==========================================\n";
    
    if (total_nodes == 4865609) {
        std::cout << "SUCCESS! Engine is correct.\n";
    } else {
        std::cout << "FAILURE. Expected 4,865,609.\n";
    }
}

int main() {
    run_perft_suite(5);
    return 0;
}