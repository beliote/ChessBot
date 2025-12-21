#include "book.h"
#include "uci.h"
#include "movegenerator.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

std::unordered_map<std::string, std::vector<std::string>> Book::book_moves;

void Book::init() {
    std::srand(std::time(nullptr));

    // --- 1. DÉBUTS (Root) ---
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq
    book_moves["rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq"] = {"e2e4", "d2d4"};

    // ==========================================
    //  SECTION E4 (King's Pawn)
    // ==========================================

    // Réponse Noire à 1. e4 : e5 (Classique), c5 (Sicilienne), e6 (Française)
    book_moves["rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq"] = {"e7e5", "c7c5", "e7e6"};

    // --- 1. e4 e5 (Open Game) ---
    // 2. Nf3
    book_moves["rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq"] = {"g1f3"};
    // 2... Nc6
    book_moves["rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq"] = {"b8c6"};
    // 3. Bb5 (Ruy Lopez) ou Bc4 (Italienne)
    book_moves["r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq"] = {"f1b5", "f1c4"};

    // Ruy Lopez: 3... a6 4. Ba4 Nf6 5. O-O
    book_moves["r1bqkbnr/1ppp1ppp/p1n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq"] = {"b5a4"}; // 4. Ba4
    book_moves["r1bqkbnr/1ppp1ppp/p1n5/4p3/B3P3/5N2/PPPP1PPP/RNBQK2R b KQkq"] = {"g8f6"}; // 4... Nf6
    book_moves["r1bqkb1r/1ppp1ppp/p1n2n2/4p3/B3P3/5N2/PPPP1PPP/RNBQK2R w KQkq"] = {"e1g1"}; // 5. O-O
    book_moves["r1bqkb1r/1ppp1ppp/p1n2n2/4p3/B3P3/5N2/PPPP1PPP/RNBQ1RK1 b kq"] = {"f8e7"}; // 5... Be7 (Closed)

    // Italienne: 3. Bc4 Bc5 4. c3 Nf6
    book_moves["r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq"] = {"f8c5", "g8f6"}; 

    // --- 1. e4 c5 (Sicilian) ---
    book_moves["rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq"] = {"g1f3"}; // 2. Nf3
    book_moves["rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq"] = {"e7e6"}; // 2... e6
    book_moves["rnbqkbnr/pp1p1ppp/4p3/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq"] = {"d2d4"}; // 3. d4
    book_moves["rnbqkbnr/pp1p1ppp/4p3/2p5/3PP3/5N2/PPP2PPP/RNBQKB1R b KQkq"] = {"c5d4"}; // 3... cxd4
    book_moves["rnbqkbnr/pp1p1ppp/4p3/8/3pP3/5N2/PPP2PPP/RNBQKB1R w KQkq"] = {"f3d4"}; // 4. Nxd4
    book_moves["rnbqkbnr/pp1p1ppp/4p3/8/3NP3/8/PPP2PPP/RNBQKB1R b KQkq"] = {"g8f6"}; // 4... Nf6
    book_moves["rnbqkb1r/pp1p1ppp/4pn2/8/3NP3/8/PPP2PPP/RNBQKB1R w KQkq"] = {"b1c3"}; // 5. Nc3
    book_moves["rnbqkb1r/pp1p1ppp/4pn2/8/3NP3/2N5/PPP2PPP/R1BQKB1R b KQkq"] = {"b8c6"}; // 5... Nc6
    book_moves["r1bqkb1r/pp1p1ppp/2n1pn2/8/3NP3/2N5/PPP2PPP/R1BQKB1R w KQkq"] = {"d4b5"};
    book_moves["r1bqkb1r/pp1p1ppp/2n1pn2/1N6/4P3/2N5/PPP2PPP/R1BQKB1R b KQkq"] = {"d7d6"};
    book_moves["r1bqkb1r/pp3ppp/2nppn2/1N6/4P3/2N5/PPP2PPP/R1BQKB1R w KQkq"] = {"c1f4"};
    book_moves["r1bqkb1r/pp3ppp/2nppn2/1N6/4PB2/2N5/PPP2PPP/R2QKB1R b KQkq"] = {"e6e5"};
    book_moves["r1bqkb1r/pp3ppp/2np1n2/1N2p3/4PB2/2N5/PPP2PPP/R2QKB1R w KQkq"] = {"f4g5"};
    book_moves["r1bqkb1r/pp3ppp/2np1n2/1N2p1B1/4P3/2N5/PPP2PPP/R2QKB1R b KQkq"] = {"a7a6"};
    book_moves["r1bqkb1r/1p3ppp/p1np1n2/1N2p1B1/4P3/2N5/PPP2PPP/R2QKB1R w KQkq"] = {"b5a3"};
    book_moves["r1bqkb1r/1p3ppp/p1np1n2/4p1B1/4P3/N1N5/PPP2PPP/R2QKB1R b KQkq"] = {"b7b5"};
    book_moves["r1bqkb1r/5ppp/p1np1n2/1p2p1B1/4P3/N1N5/PPP2PPP/R2QKB1R w KQkq"] = {"c3d5"};
    book_moves["r1bqkb1r/5ppp/p1np1n2/1p1Np1B1/4P3/N7/PPP2PPP/R2QKB1R b KQkq"] = {"f8e7"};
    book_moves["r1bqk2r/4bppp/p1np1n2/1p1Np1B1/4P3/N7/PPP2PPP/R2QKB1R w KQkq"] = {"g5f6"};
    book_moves["r1bqk2r/4bppp/p1np1B2/1p1Np3/4P3/N7/PPP2PPP/R2QKB1R b KQkq"] = {"e7f6"};
    book_moves["r1bqk2r/5ppp/p1np1b2/1p1Np3/4P3/N7/PPP2PPP/R2QKB1R w KQkq"] = {"c2c3"};
    book_moves["r1bqk2r/5ppp/p1np1b2/1p1Np3/4P3/N1P5/PP3PPP/R2QKB1R b KQkq"] = {"a8b8"};
    book_moves["1rbqk2r/5ppp/p1np1b2/1p1Np3/4P3/N1P5/PP3PPP/R2QKB1R w KQk"] = {"a3c2"};
    book_moves["1rbqk2r/5ppp/p1np1b2/1p1Np3/4P3/2P5/PPN2PPP/R2QKB1R b KQk"] = {"f6g5"};
    book_moves["1rbqk2r/5ppp/p1np4/1p1Np1b1/4P3/2P5/PPN2PPP/R2QKB1R w KQk"] = {"a2a4"};
    book_moves["1rbqk2r/5ppp/p1np4/1p1Np1b1/P3P3/2P5/1PN2PPP/R2QKB1R b KQk"] = {"b5a4"};
    book_moves["1rbqk2r/5ppp/p1np4/3Np1b1/p3P3/2P5/1PN2PPP/R2QKB1R w KQk"] = {"c2b4"};
    book_moves["1rbqk2r/5ppp/p1np4/3Np1b1/pN2P3/2P5/1P3PPP/R2QKB1R b KQk"] = {"c6b4"};
    book_moves["1rbqk2r/5ppp/p2p4/3Np1b1/pn2P3/2P5/1P3PPP/R2QKB1R w KQk"] = {"c3b4"};
    book_moves["1rbqk2r/5ppp/p2p4/3Np1b1/pP2P3/8/1P3PPP/R2QKB1R b KQk"] = {"e8g8"};
    book_moves["1rbq1rk1/5ppp/p2p4/3Np1b1/pP2P3/8/1P3PPP/R2QKB1R w KQ"] = {"a1a4"};
    book_moves["1rbq1rk1/5ppp/p2p4/3Np1b1/RP2P3/8/1P3PPP/3QKB1R b K"] = {"a6a5"};
    book_moves["1rbq1rk1/5ppp/3p4/p2Np1b1/RP2P3/8/1P3PPP/3QKB1R w K"] = {"h2h4"};
    book_moves["1rbq1rk1/5ppp/3p4/p2Np1b1/RP2P2P/8/1P3PP1/3QKB1R b K"] = {"g5h6"};
    book_moves["1rbq1rk1/5ppp/3p3b/p2Np3/RP2P2P/8/1P3PP1/3QKB1R w K"] = {"b4b5"};
    book_moves["1rbq1rk1/5ppp/3p3b/pP1Np3/R3P2P/8/1P3PP1/3QKB1R b K"] = {"c8d7"};
    book_moves["1r1q1rk1/3b1ppp/3p3b/pP1Np3/R3P2P/8/1P3PP1/3QKB1R w K"] = {"d5c3"};
    book_moves["1r1q1rk1/3b1ppp/3p3b/pP2p3/R3P2P/2N5/1P3PP1/3QKB1R b K"] = {"d6d5"};
    book_moves["1r1q1rk1/3b1ppp/7b/pP1pp3/R3P2P/2N5/1P3PP1/3QKB1R w K"] = {"e4d5"};
    book_moves["1r1q1rk1/3b1ppp/7b/pP1Pp3/R6P/2N5/1P3PP1/3QKB1R b K"] = {"g8h8"};
    book_moves["1r1q1r1k/3b1ppp/7b/pP1Pp3/R6P/2N5/1P3PP1/3QKB1R w K"] = {"f1e2"};
    book_moves["1r1q1r1k/3b1ppp/7b/pP1Pp3/R6P/2N5/1P2BPP1/3QK2R b K"] = {"f7f5"};
    book_moves["1r1q1r1k/3b2pp/7b/pP1Ppp2/R6P/2N5/1P2BPP1/3QK2R w K"] = {"e1g1"};
    book_moves["1r1q1r1k/3b2pp/7b/pP1Ppp2/R6P/2N5/1P2BPP1/3Q1RK1 b"] = {"g7g5"};







    // Najdorf: 5... a6
    book_moves["rnbqkb1r/pp2pppp/3p1n2/8/3NP3/2N5/PPP2PPP/R1BQKB1R b KQkq"] = {"a7a6", "g7g6"};

    // --- 1. e4 e6 (French) ---
    book_moves["rnbqkbnr/pppp1ppp/4p3/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq"] = {"d2d4"}; // 2. d4
    book_moves["rnbqkbnr/pppp1ppp/4p3/8/3PP3/8/PPP2PPP/RNBQKBNR b KQkq"] = {"d7d5"}; // 2... d5

    // ==========================================
    //  SECTION D4 (Queen's Pawn)
    // ==========================================

    // Réponse Noire à 1. d4 : d5 (QGD/Slav) ou Nf6 (Indian)
    book_moves["rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq"] = {"d7d5", "g8f6"};

    // --- 1. d4 d5 (Queen's Gambit) ---
    book_moves["rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq"] = {"c2c4"}; // 2. c4
    book_moves["rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq"] = {"e7e6", "c7c6"}; // 2... e6/c6
    
    // QGD Line: 3. Nc3 Nf6 4. Bg5 Be7
    book_moves["rnbqkbnr/ppp2ppp/4p3/3p4/2PP4/8/PP2PPPP/RNBQKBNR w KQkq"] = {"b1c3"}; // 3. Nc3
    book_moves["rnbqkbnr/ppp2ppp/4p3/3p4/2PP4/2N5/PP2PPPP/R1BQKBNR b KQkq"] = {"g8f6"}; // 3... Nf6
    book_moves["rnbqkb1r/ppp2ppp/4pn2/3p4/2PP4/2N5/PP2PPPP/R1BQKBNR w KQkq"] = {"c1g5", "g1f3"}; // 4. Bg5/Nf3

    // --- 1. d4 Nf6 (Indian Defenses) ---
    // 2. c4 g6 (King's Indian) ou e6 (Nimzo/Queen's Indian)
    book_moves["rnbqkb1r/pppppppp/5n2/8/3P4/8/PPP1PPPP/RNBQKBNR w KQkq"] = {"c2c4"}; // 2. c4
    book_moves["rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b KQkq"] = {"g7g6", "e7e6"};

    // KID: 3. Nc3 Bg7 4. e4 d6
    book_moves["rnbqkb1r/pppppp1p/5np1/8/2PP4/8/PP2PPPP/RNBQKBNR w KQkq"] = {"b1c3"}; // 3. Nc3
    book_moves["rnbqkb1r/pppppp1p/5np1/8/2PP4/2N5/PP2PPPP/R1BQKBNR b KQkq"] = {"f8g7"}; // 3... Bg7
    book_moves["rnbqk2r/ppppppbp/5np1/8/2PP4/2N5/PP2PPPP/R1BQKBNR w KQkq"] = {"e2e4"}; // 4. e4
    book_moves["rnbqk2r/ppppppbp/5np1/8/2PPP3/2N5/PP3PPP/R1BQKBNR b KQkq"] = {"d7d6"}; // 4... d6
}

std::string Book::clean_fen(const std::string& full_fen) {
    std::string clean = "";
    int spaces = 0;
    for (char c : full_fen) {
        if (c == ' ') spaces++;
        if (spaces >= 3) break;
        clean += c;
    }
    return clean;
}

Move Book::get_book_move(const Board& board) {
    std::string fen = board.get_fen();
    std::string key = clean_fen(fen);

    if (book_moves.find(key) != book_moves.end()) {
        const auto& moves = book_moves[key];
        int index = std::rand() % moves.size();
        std::string move_str = moves[index];
        
        // Parsing simplifié
        int f1 = move_str[0] - 'a';
        int r1 = 8 - (move_str[1] - '0');
        int f2 = move_str[2] - 'a';
        int r2 = 8 - (move_str[3] - '0');
        
        Square from = (Square)(r1 * 8 + f1);
        Square to = (Square)(r2 * 8 + f2);
        
        std::vector<Move> legal_moves;
        MoveGenerator::generate_legal_moves(board, legal_moves);
        
        for (const auto& m : legal_moves) {
            if (get_from_sq(m) == from && get_to_sq(m) == to) {
                return m;
            }
        }
    }
    
    return MOVE_NONE;
}