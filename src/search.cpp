#include "../include/search.h"
#include "../include/types.h"
#include "../include/book.h"
#include "../include/tt.h" 
#include <algorithm>
#include <iostream>
#include <vector>

// Initialisation des membres statiques
uint64_t Search::nodes_searched = 0;
int Search::max_depth_reached = 0;
std::chrono::steady_clock::time_point Search::start_time;
int Search::allotted_time_ms = 0;
bool Search::stop_flag = false;

// Tableaux statiques
Move Search::killer_moves[64][2] = {};
int Search::history[12][64] = {};

void Search::reset_stats() {
    nodes_searched = 0;
    max_depth_reached = 0;
    stop_flag = false;
}

void Search::stop_search() { stop_flag = true; }
void Search::set_time_limit(int time_limit_ms) { allotted_time_ms = time_limit_ms; }

bool Search::is_time_up() {
    if (allotted_time_ms <= 0) return false;
    if ((nodes_searched & 2047) == 0) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
        if (elapsed >= allotted_time_ms) return true;
    }
    return false;
}

// Fonction de score corrigée : utilise les membres statiques de la classe
int Search::score_move(Move move, const Board& board, int ply, Move tt_move) {
    if (move == tt_move) return 20000; // TT Move en premier

    int captured = board.piece_at(get_to_sq(move));
    if (captured != NO_PIECE && captured <= 11) {
        int victim = Evaluation::get_piece_value(captured);
        int attacker = Evaluation::get_piece_value(board.piece_at(get_from_sq(move)));
        return 10000 + victim * 10 - attacker;
    }
    
    if (ply < 64) {
        if (move == killer_moves[ply][0]) return 9000;
        if (move == killer_moves[ply][1]) return 8000;
    }
    
    int p = board.piece_at(get_from_sq(move));
    if (p >= 0 && p <= 11) return history[p][get_to_sq(move)];
    
    return 0;
}

int Search::quiescence(Board& board, int alpha, int beta) {
    if (is_time_up()) stop_flag = true;
    if (stop_flag) return 0;
    nodes_searched++;

    int stand_pat = Evaluation::evaluate(board);
    if (stand_pat >= beta) return beta;
    if (alpha < stand_pat) alpha = stand_pat;

    std::vector<Move> moves;
    moves.reserve(64);
    MoveGenerator::generate_legal_moves(board, moves);

    std::vector<std::pair<int, Move>> captures;
    captures.reserve(moves.size());
    for (Move m : moves) {
        if (board.piece_at(get_to_sq(m)) != NO_PIECE) 
            // Appel corrigé : pas besoin de passer killer/history, et tt_move est NONE
            captures.push_back({score_move(m, board, 0, MOVE_NONE), m});
    }
    std::sort(captures.rbegin(), captures.rend());

    for (const auto& pair : captures) {
        Board next = board;
        next.make_move(pair.second);
        int score = -quiescence(next, -beta, -alpha);
        
        if (stop_flag) return 0;
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

int Search::negamax(Board& board, int depth, int alpha, int beta, int ply) {
    if (is_time_up()) stop_flag = true;
    if (stop_flag) return 0;

    // TT Probe
    int tt_score = 0;
    Move tt_move = MOVE_NONE;
    int tt_depth = 0;
    int tt_flag = 0;
    if (TT::probe(board.hash_key, tt_score, tt_move, tt_depth, tt_flag, depth, alpha, beta)) {
        return tt_score;
    }

    bool in_check = board.is_in_check();
    if (in_check) depth++;

    if (depth <= 0) return quiescence(board, alpha, beta);
    nodes_searched++;

    if (depth >= 3 && !in_check && ply > 0) {
        Board copy = board;
        copy.make_null_move();
        int score = -negamax(copy, depth - 3, -beta, -beta + 1, ply + 1);
        if (stop_flag) return 0;
        if (score >= beta) return beta;
    }

    std::vector<Move> moves;
    moves.reserve(256);
    MoveGenerator::generate_legal_moves(board, moves);

    if (moves.empty()) {
        if (in_check) return -49000 + ply;
        return 0;
    }

    std::vector<std::pair<int, Move>> sorted_moves;
    sorted_moves.reserve(moves.size());
    for (Move m : moves) {
        // Appel corrigé : on passe ply et tt_move
        sorted_moves.push_back({score_move(m, board, ply, tt_move), m});
    }
    std::sort(sorted_moves.rbegin(), sorted_moves.rend());

    int best_score = -50000;
    Move best_move_found = MOVE_NONE;
    int start_alpha = alpha;
    
    for (size_t i = 0; i < sorted_moves.size(); ++i) {
        Move move = sorted_moves[i].second;
        Board next = board;
        next.make_move(move);
        
        int score;
        if (i == 0) {
            score = -negamax(next, depth - 1, -beta, -alpha, ply + 1);
        } else {
            score = -negamax(next, depth - 1, -alpha - 1, -alpha, ply + 1);
            if (score > alpha && score < beta)
                score = -negamax(next, depth - 1, -beta, -alpha, ply + 1);
        }
        
        if (stop_flag) return 0;
        
        if (score > best_score) {
            best_score = score;
            best_move_found = move;
        }
        
        if (score > alpha) {
            alpha = score;
            if (ply < 64 && board.piece_at(get_to_sq(move)) == NO_PIECE) {
                killer_moves[ply][1] = killer_moves[ply][0];
                killer_moves[ply][0] = move;
                int p = board.piece_at(get_from_sq(move));
                if (p != NO_PIECE) history[p][get_to_sq(move)] += depth * depth;
            }
        }
        if (alpha >= beta) break;
    }

    int flag = TT_ALPHA;
    if (best_score >= beta) flag = TT_BETA;
    else if (best_score > start_alpha) flag = TT_EXACT;
    
    TT::store(board.hash_key, best_score, best_move_found, depth, flag, ply);

    return best_score;
}

Move Search::get_best_move(Board& board, int max_depth, int time_limit_ms) {
    Move book_move = Book::get_book_move(board); 
    if (book_move != MOVE_NONE) {
        std::cout << "info string Book move" << std::endl;
        return book_move;
    }

    reset_stats();
    start_time = std::chrono::steady_clock::now();
    allotted_time_ms = (time_limit_ms > 50) ? time_limit_ms - 50 : time_limit_ms;

    std::vector<Move> root_moves;
    MoveGenerator::generate_legal_moves(board, root_moves);
    if (root_moves.empty()) return MOVE_NONE;

    std::vector<std::pair<int, Move>> sorted_root;
    for (Move m : root_moves) {
        // Appel corrigé : ply=0, pas de tt_move initial connu ici (ou on pourrait probe)
        sorted_root.push_back({score_move(m, board, 0, MOVE_NONE), m});
    }
    std::sort(sorted_root.rbegin(), sorted_root.rend());

    Move best_move = sorted_root[0].second;

    for (int depth = 1; depth <= max_depth; ++depth) {
        if (allotted_time_ms > 0 && depth > 1) {
             auto now = std::chrono::steady_clock::now();
             auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
             if (elapsed > allotted_time_ms * 0.6) break; 
        }

        int alpha = -50000;
        int beta = 50000;
        int best_score_depth = -50000;
        Move current_best_move = MOVE_NONE;

        for (auto& pair : sorted_root) {
            Move m = pair.second;
            Board next = board;
            next.make_move(m);
            int val = -negamax(next, depth - 1, -beta, -alpha, 1);
            
            if (stop_flag) break;
            
            if (val > best_score_depth) {
                best_score_depth = val;
                current_best_move = m;
            }
            if (val > alpha) alpha = val;
        }

        if (!stop_flag) {
            best_move = current_best_move;
            // Réorganisation du tri racine
            for (auto& pair : sorted_root) {
                if (pair.second == best_move) {
                    pair.first = 100000; 
                } else {
                    pair.first = score_move(pair.second, board, 0, MOVE_NONE);
                }
            }
            std::sort(sorted_root.rbegin(), sorted_root.rend());

            auto t = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start_time).count();
            if (t == 0) t = 1;
            
            std::cout << "info depth " << depth << " score cp " << best_score_depth 
                      << " nodes " << nodes_searched << " time " << t 
                      << " nps " << (nodes_searched * 1000 / t) << std::endl;
        } else break;
    }
    return best_move;
}