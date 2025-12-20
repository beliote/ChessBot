#pragma once

#include "board.h"
#include "search.h"
#include <string>
#include <vector>

class Uci {
public:
    Uci();
    ~Uci();
    
    // Main UCI loop
    void loop();
    
private:
    Board board;
    bool quit_flag;
    
    // UCI command handlers
    void handle_uci();
    void handle_isready();
    void handle_ucinewgame();
    void handle_position(const std::string& command);
    void handle_go(const std::string& command);
    void handle_quit();
    
    // Helper functions
    Square parse_square(const std::string& square_str);
    Move parse_move(const std::string& move_str);
    std::string move_to_string(Move move);
    
    // Time management
    int calculate_search_time(int time_left, int increment);
};

