#include "board.h"
#include "movegenerator.h"
#include "bitboard.h"
#include "eval.h"
#include "search.h"
#include <iostream>
#include <chrono>
#include <iomanip>

int main() {
    // Initialize engine
    Bitboards::init();
    MoveGenerator::init();
    
    std::cout << "==========================================\n";
    std::cout << "  CHESS ENGINE - AI SEARCH TEST\n";
    std::cout << "==========================================\n\n";
    
    // Set up a tricky position (mate in 2 puzzle)
    // This is a classic puzzle: White to move and mate in 2
    // Position: r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1
    // Or use a simpler test position
    Board board;
    board.set_fen("r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1");
    
    std::cout << "Initial Position:\n";
    board.print();
    std::cout << "\n";
    
    // Evaluate initial position
    int eval = Evaluation::evaluate(board);
    std::cout << "Static Evaluation: " << eval << " cp\n";
    std::cout << "(Positive = advantage for side to move, Negative = disadvantage)\n\n";
    
    // Search for best move
    std::cout << "Searching for best move (depth 5)...\n";
    std::cout << "------------------------------------------\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    Move best_move = Search::get_best_move(board, 5);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "------------------------------------------\n";
    std::cout << "Search completed in " << duration.count() << " ms\n";
    std::cout << "Nodes searched: " << Search::nodes_searched << "\n";
    std::cout << "Max depth reached: " << Search::max_depth_reached << "\n\n";
    
    if (best_move == MOVE_NONE) {
        std::cout << "No legal moves found!\n";
        return 0;
    }
    
    // Display best move
    Square from = get_from_sq(best_move);
    Square to = get_to_sq(best_move);
    char f1 = 'a' + file_of(from);
    char r1 = '8' - rank_of(from);
    char f2 = 'a' + file_of(to);
    char r2 = '8' - rank_of(to);
    
    std::cout << "Best Move: " << f1 << r1 << f2 << r2;
    
    // Show move type if special
    int move_type = get_move_type(best_move);
    if (move_type == MOVE_TYPE_PROMOTION) {
        int promo = get_promotion(best_move);
        char promo_chars[] = {'n', 'b', 'r', 'q'};
        std::cout << promo_chars[promo];
    } else if (move_type == MOVE_TYPE_CASTLING) {
        std::cout << " (castling)";
    } else if (move_type == MOVE_TYPE_EN_PASSANT) {
        std::cout << " (en passant)";
    }
    std::cout << "\n\n";
    
    // Make the move and show resulting position
    board.make_move(best_move);
    std::cout << "Position after best move:\n";
    board.print();
    std::cout << "\n";
    
    // Evaluate new position
    eval = Evaluation::evaluate(board);
    std::cout << "New Static Evaluation: " << eval << " cp\n";
    
    std::cout << "\n==========================================\n";
    
    return 0;
}
