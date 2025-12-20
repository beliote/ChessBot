#include "uci.h"
#include "movegenerator.h"
#include "bitboard.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>

Uci::Uci() : quit_flag(false) {
    // Board is initialized by default constructor
}

Uci::~Uci() {
}

void Uci::loop() {
    std::string line;
    
    while (std::getline(std::cin, line) && !quit_flag) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (line.empty()) continue;
        
        // Parse command
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        
        if (command == "uci") {
            handle_uci();
        } else if (command == "isready") {
            handle_isready();
        } else if (command == "ucinewgame") {
            handle_ucinewgame();
        } else if (command == "position") {
            handle_position(line);
        } else if (command == "go") {
            handle_go(line);
        } else if (command == "quit") {
            handle_quit();
            break;
        } else if (command == "stop") {
            // Stop the search
            Search::stop_search();
            // Note: bestmove will be output when search completes or times out
        } else if (command == "debug") {
            // Ignore debug command
        } else {
            // Unknown command - ignore
        }
    }
}

void Uci::handle_uci() {
    std::cout << "id name CppChess Engine" << std::endl;
    std::cout << "id author Your Name" << std::endl;
    std::cout << "uciok" << std::endl;
}

void Uci::handle_isready() {
    std::cout << "readyok" << std::endl;
}

void Uci::handle_ucinewgame() {
    // Reset board to starting position
    board.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Search::reset_stats();
}

void Uci::handle_position(const std::string& command) {
    std::istringstream iss(command);
    std::string token;
    iss >> token; // Skip "position"
    
    iss >> token;
    
    if (token == "startpos") {
        // Set starting position
        board.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    } else if (token == "fen") {
        // Parse FEN string
        std::string fen;
        std::string part;
        int parts = 0;
        while (iss >> part && parts < 6) {
            if (parts > 0) fen += " ";
            fen += part;
            parts++;
        }
        board.set_fen(fen);
    }
    
    // Parse moves
    if (iss >> token && token == "moves") {
        std::string move_str;
        while (iss >> move_str) {
            Move move = parse_move(move_str);
            if (move != MOVE_NONE) {
                board.make_move(move);
            }
        }
    }
}

void Uci::handle_go(const std::string& command) {
    std::istringstream iss(command);
    std::string token;
    iss >> token; // Skip "go"
    
    int depth = 5; // Default depth
    int wtime = 0, btime = 0, winc = 0, binc = 0;
    int movetime = 0;
    bool infinite = false;
    
    // Parse go parameters
    while (iss >> token) {
        if (token == "depth") {
            iss >> depth;
        } else if (token == "wtime") {
            iss >> wtime;
        } else if (token == "btime") {
            iss >> btime;
        } else if (token == "winc") {
            iss >> winc;
        } else if (token == "binc") {
            iss >> binc;
        } else if (token == "movetime") {
            iss >> movetime;
        } else if (token == "infinite") {
            infinite = true;
        }
    }
    
    // Calculate search time
    int search_time_ms = 0;
    if (movetime > 0) {
        search_time_ms = movetime;
    } else if (!infinite) {
        if (board.side_to_move == WHITE && wtime > 0) {
            search_time_ms = calculate_search_time(wtime, winc);
        } else if (board.side_to_move == BLACK && btime > 0) {
            search_time_ms = calculate_search_time(btime, binc);
        }
    }
    
    // Perform search with iterative deepening
    Search::reset_stats();
    Move best_move = MOVE_NONE;
    
    if (infinite || search_time_ms == 0) {
        // No time limit: use fixed depth (or very high time limit)
        best_move = Search::get_best_move(board, depth, 0);
    } else {
        // Time-limited search with iterative deepening
        best_move = Search::get_best_move(board, 64, search_time_ms);
    }
    
    // Output best move
    if (best_move == MOVE_NONE) {
        std::cout << "bestmove 0000" << std::endl;
    } else {
        std::cout << "bestmove " << move_to_string(best_move) << std::endl;
    }
}

void Uci::handle_quit() {
    quit_flag = true;
}

Square Uci::parse_square(const std::string& square_str) {
    if (square_str.length() < 2) return NO_SQ;
    
    char file_char = square_str[0];
    char rank_char = square_str[1];
    
    if (file_char < 'a' || file_char > 'h') return NO_SQ;
    if (rank_char < '1' || rank_char > '8') return NO_SQ;
    
    int file = file_char - 'a';
    int rank_number = rank_char - '0';
    
    // Convert UCI rank to internal rank
    // UCI: rank 1 = bottom, rank 8 = top
    // Internal: rank 0 = top (A8), rank 7 = bottom (H1)
    int rank = 8 - rank_number;
    
    return square_from_coords(rank, file);
}

Move Uci::parse_move(const std::string& move_str) {
    if (move_str.length() < 4) return MOVE_NONE;
    
    // Parse from and to squares
    std::string from_str = move_str.substr(0, 2);
    std::string to_str = move_str.substr(2, 2);
    
    Square from = parse_square(from_str);
    Square to = parse_square(to_str);
    
    if (from == NO_SQ || to == NO_SQ) return MOVE_NONE;
    
    // Check for promotion
    int move_type = MOVE_TYPE_NORMAL;
    int promotion = 0;
    
    if (move_str.length() >= 5) {
        move_type = MOVE_TYPE_PROMOTION;
        char promo_char = move_str[4];
        switch (promo_char) {
            case 'n': promotion = 0; break; // Knight
            case 'b': promotion = 1; break; // Bishop
            case 'r': promotion = 2; break; // Rook
            case 'q': promotion = 3; break; // Queen
            default: move_type = MOVE_TYPE_NORMAL; break;
        }
    }
    
    // Verify the move is legal by generating all moves and checking
    // This ensures we get the correct move type (castling, en passant, etc.)
    std::vector<Move> legal_moves;
    MoveGenerator::generate_legal_moves(board, legal_moves);
    
    // Find matching legal move
    for (Move legal_move : legal_moves) {
        if (get_from_sq(legal_move) == from && 
            get_to_sq(legal_move) == to) {
            // If promotion was specified, check promotion piece matches
            if (move_type == MOVE_TYPE_PROMOTION) {
                if (get_move_type(legal_move) == MOVE_TYPE_PROMOTION &&
                    get_promotion(legal_move) == promotion) {
                    return legal_move;
                }
            } else {
                // For non-promotion moves, return the legal move (it has correct type)
                return legal_move;
            }
        }
    }
    
    return MOVE_NONE;
}

std::string Uci::move_to_string(Move move) {
    if (move == MOVE_NONE) return "0000";
    
    Square from = get_from_sq(move);
    Square to = get_to_sq(move);
    
    // Convert internal square to UCI notation
    int file = file_of(from);
    int rank = rank_of(from);
    int uci_rank = 8 - rank; // Convert internal rank to UCI rank
    
    char file_char = 'a' + file;
    char rank_char = '0' + uci_rank;
    
    std::string result;
    result += file_char;
    result += rank_char;
    
    file = file_of(to);
    rank = rank_of(to);
    uci_rank = 8 - rank;
    
    file_char = 'a' + file;
    rank_char = '0' + uci_rank;
    
    result += file_char;
    result += rank_char;
    
    // Add promotion character if needed
    int move_type = get_move_type(move);
    if (move_type == MOVE_TYPE_PROMOTION) {
        int promo = get_promotion(move);
        char promo_chars[] = {'n', 'b', 'r', 'q'};
        if (promo >= 0 && promo < 4) {
            result += promo_chars[promo];
        }
    }
    
    return result;
}

int Uci::calculate_search_time(int time_left, int increment) {
    // Simple time management: use time_left / 25
    // Add increment for safety
    int base_time = time_left / 25;
    int safe_time = base_time + increment;
    
    // Don't use more than 90% of remaining time
    int max_time = (time_left * 9) / 10;
    
    return std::min(safe_time, max_time);
}

