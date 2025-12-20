#include "uci.h"
#include "movegenerator.h"
#include "bitboard.h"
#include "zobrist.h"
#include "tt.h"
#include <iostream>

int main() {
    // Initialize engine components
    Bitboards::init();
    MoveGenerator::init();
    Zobrist::init_zobrist();
    TT::init(64);  // Initialize TT with 64MB
    
    // Start UCI protocol loop
    Uci uci;
    uci.loop();
    
    return 0;
}
