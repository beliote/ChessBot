#include "uci.h"
#include "movegenerator.h"
#include "bitboard.h"
#include "zobrist.h"
#include "tt.h"
#include "book.h"
#include "selfplay.h"
#include <string>
#include <cstring>
#include <iostream>

int main(int argc, char* argv[]) {
    // Initialize engine components
    Bitboards::init();
    MoveGenerator::init();
    Zobrist::init_zobrist();
    Book::init();
    TT::init(64);  // Initialize TT with 64MB

    // --- MODE SELF-PLAY ---
    if (argc > 1 && std::string(argv[1]) == "selfplay") {
        // Lance 1000 parties à profondeur 2
        // Vous pouvez changer ces valeurs
        SelfPlay::start_training(1000, 2);
        return 0;
    }
    
    // Start UCI protocol loop
    Uci uci;
    uci.loop();
    
    return 0;
}
