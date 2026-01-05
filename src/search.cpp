#include "../include/search.h"
#include "../include/types.h"
#include "../include/book.h"
#include "../include/tt.h" 
#include <algorithm>
#include <iostream>
#include <vector>

// Init static
uint64_t Search::nodes_searched = 0;
int Search::max_depth_reached = 0;
std::chrono::steady_clock::time_point Search::start_time;
int Search::allotted_time_ms = 0;
bool Search::stop_flag = false;

Move Search::killer_moves[64][2] = {};
int Search::history[12][64] = {};

// --- NOUVEAU : Historique de la branche de recherche ---
// Sert à détecter les répétitions DANS le calcul (ex: perpétuel)
static uint64_t search_ply_history[256]; 

void Search::reset_stats() {
    nodes_searched = 0;
    max_depth_reached = 0;
    stop_flag = false;
}

void Search::stop_search() { stop_flag = true; }
void Search::set_time_limit(int time_limit_ms) { allotted_time_ms = time_limit_ms; }

bool Search::is_time_up() {
    if (allotted_time_ms <= 0) return false;
    if ((nodes_searched & 4095) == 0) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() >= allotted_time_ms) 
            return true;
    }
    return false;
}

// Vérifie si la position actuelle est une répétition dans la branche de recherche
bool is_repetition(uint64_t hash, int ply) {
    // On regarde les positions précédentes dans la ligne actuelle
    // On remonte par pas de 2 (car une répétition survient après un coup à nous + un coup à eux)
    for (int i = ply - 2; i >= 0; i -= 2) {
        if (search_ply_history[i] == hash) return true;
    }
    return false;
}

int Search::score_move(Move move, const Board& board, int ply, Move tt_move) {
    if (move == tt_move) return 20000;

    int captured = board.piece_at(get_to_sq(move));
    if (captured != NO_PIECE && captured <= 11) {
        int victim = Evaluation::get_piece_value((Piece)captured);
        int attacker = Evaluation::get_piece_value((Piece)board.piece_at(get_from_sq(move)));
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

bool is_legal(const Board& board) {
    Color mover = (Color)(1 - board.side_to_move);
    int king_idx = (mover == WHITE) ? WHITE_KING : BLACK_KING;
    Bitboard king_bb = board.pieces[king_idx];
    if (king_bb == 0) return false;
    int king_sq = __builtin_ctzll(king_bb);
    return !board.is_square_attacked(king_sq, board.side_to_move);
}

int Search::quiescence(Board& board, int alpha, int beta, int ply) {
    if (is_time_up()) stop_flag = true;
    if (stop_flag) return 0;
    nodes_searched++;

    // En Quiescence, on ne vérifie pas strictement la répétition pour la vitesse,
    // mais on enregistre quand même le hash pour negamax.
    if (ply < 256) search_ply_history[ply] = board.hash_key;

    int stand_pat = Evaluation::evaluate(board);
    if (stand_pat >= beta) return beta;
    if (alpha < stand_pat) alpha = stand_pat;

    std::vector<Move> moves;
    moves.reserve(64);
    MoveGenerator::generate_pseudo_moves(board, moves);

    std::vector<std::pair<int, Move>> captures;
    captures.reserve(moves.size());
    for (Move m : moves) {
        if (board.piece_at(get_to_sq(m)) != NO_PIECE) 
            captures.push_back({score_move(m, board, 0, MOVE_NONE), m});
    }
    std::sort(captures.rbegin(), captures.rend());

    for (const auto& pair : captures) {
        Move move = pair.second;
        Board next = board;
        next.make_move(move);
        
        if (!is_legal(next)) continue;

        int score = -quiescence(next, -beta, -alpha, ply + 1);
        
        if (stop_flag) return 0;
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

int Search::negamax(Board& board, int depth, int alpha, int beta, int ply) {
    if (is_time_up()) stop_flag = true;
    if (stop_flag) return 0;

    // --- DETECTION REPETITION ---
    // Si on rencontre la même position dans cette branche, c'est NUL (0).
    // Si on est gagnant (+500), le moteur verra que 0 < 500 et évitera cette branche.
    if (ply > 0 && is_repetition(board.hash_key, ply)) {
        return 0; 
    }
    // On enregistre la position actuelle dans l'historique
    if (ply < 256) search_ply_history[ply] = board.hash_key;
    // ----------------------------

    int tt_score = 0; Move tt_move = MOVE_NONE; int tt_depth = 0; int tt_flag = 0;
    if (TT::probe(board.hash_key, tt_score, tt_move, tt_depth, tt_flag, depth, alpha, beta)) {
        // Attention: ne pas retourner un score TT si c'est une répétition qu'on n'a pas vue
        // Mais pour l'instant, on garde simple.
        return tt_score;
    }

    bool in_check = board.is_in_check();
    if (in_check) depth++;

    if (depth <= 0) return quiescence(board, alpha, beta, ply);
    nodes_searched++;

    int static_eval = Evaluation::evaluate(board);

    // 1. REVERSE FUTILITY PRUNING (RFP)
    if (depth <= 7 && !in_check && ply > 0) {
        int margin = 80 * depth; 
        if (static_eval - margin >= beta) return static_eval;
    }

    // 2. NULL MOVE PRUNING
    if (depth >= 3 && !in_check && ply > 0 && static_eval >= beta) {
        Board copy = board;
        copy.make_null_move();
        // Null move ne change pas le hash de la même façon, attention à l'historique (on ignore ici)
        
        int R = 2; 
        if (depth > 6) R = 3;
        
        int score = -negamax(copy, depth - 1 - R, -beta, -beta + 1, ply + 1);
        if (stop_flag) return 0;
        if (score >= beta) return beta;
    }

    // 3. RAZORING
    if (depth <= 3 && !in_check && ply > 0) {
        int margin = 300 * depth;
        if (static_eval + margin < alpha) {
            int q_score = quiescence(board, alpha, beta, ply);
            if (q_score < alpha) return q_score;
        }
    }

    std::vector<Move> moves;
    moves.reserve(256);
    MoveGenerator::generate_pseudo_moves(board, moves);

    std::vector<std::pair<int, Move>> sorted_moves;
    sorted_moves.reserve(moves.size());
    for (Move m : moves) {
        sorted_moves.push_back({score_move(m, board, ply, tt_move), m});
    }
    std::sort(sorted_moves.rbegin(), sorted_moves.rend());

    int best_score = -50000;
    Move best_move_found = MOVE_NONE;
    int legal_moves_count = 0;
    
    for (size_t i = 0; i < sorted_moves.size(); ++i) {
        Move move = sorted_moves[i].second;
        Board next = board;
        next.make_move(move);
        
        if (!is_legal(next)) continue;
        legal_moves_count++;

        // 4. LATE MOVE REDUCTION (LMR) - Version Sécurisée
        int reduction = 0;
        bool is_capture = (board.piece_at(get_to_sq(move)) != NO_PIECE);
        bool is_killer = (move == killer_moves[ply][0] || move == killer_moves[ply][1]);
        bool is_promotion = (get_move_type(move) == MOVE_TYPE_PROMOTION);

        if (depth >= 3 && i > 3 && !in_check && !is_capture && !is_killer && !is_promotion) {
             reduction = 1;
             if (i > 8 && depth > 6) reduction = 2;
        }

        int score;
        if (legal_moves_count == 1) {
            score = -negamax(next, depth - 1, -beta, -alpha, ply + 1);
        } else {
            score = -negamax(next, depth - 1 - reduction, -alpha - 1, -alpha, ply + 1);
            if (score > alpha && reduction > 0) {
                 score = -negamax(next, depth - 1, -alpha - 1, -alpha, ply + 1);
            }
            if (score > alpha && score < beta) {
                score = -negamax(next, depth - 1, -beta, -alpha, ply + 1);
            }
        }
        
        if (stop_flag) return 0;
        
        if (score > best_score) {
            best_score = score;
            best_move_found = move;
        }
        
        if (score > alpha) {
            alpha = score;
            if (ply < 64 && !is_capture) {
                killer_moves[ply][1] = killer_moves[ply][0];
                killer_moves[ply][0] = move;
                int p = board.piece_at(get_from_sq(move));
                if (p != NO_PIECE) history[p][get_to_sq(move)] += depth * depth;
            }
        }
        if (alpha >= beta) break;
    }

    if (legal_moves_count == 0) {
        if (in_check) return -49000 + ply;
        // Contempt: On retourne 0 pour le pat.
        // Si le moteur gagne (+900), 0 est mauvais -> il évitera le pat.
        // Si le moteur perd (-900), 0 est bon -> il cherchera le pat.
        return 0;
    }

    int flag = TT_ALPHA;
    if (best_score >= beta) flag = TT_BETA;
    else if (best_score > alpha) flag = TT_EXACT;
    
    TT::store(board.hash_key, best_score, best_move_found, depth, flag, ply);

    return best_score;
}

Move Search::get_best_move(Board& board, int max_depth, int time_limit_ms) {
    Move book_move = Book::get_book_move(board); 
    if (book_move != MOVE_NONE) {
        std::cout << "info string Book move joue !" << std::endl;
        return book_move;
    }

    reset_stats();
    // Reset historique de recherche
    for(int i=0; i<256; ++i) search_ply_history[i] = 0;
    
    start_time = std::chrono::steady_clock::now();
    allotted_time_ms = (time_limit_ms > 50) ? time_limit_ms - 50 : time_limit_ms;

    std::vector<Move> root_moves;
    MoveGenerator::generate_legal_moves(board, root_moves);
    if (root_moves.empty()) return MOVE_NONE;

    std::vector<std::pair<int, Move>> sorted_root;
    for (Move m : root_moves) {
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

        // On enregistre la racine dans l'historique (ply 0)
        search_ply_history[0] = board.hash_key;

        for (auto& pair : sorted_root) {
            Move m = pair.second;
            Board next = board;
            next.make_move(m);
            // On appelle negamax avec ply = 1
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
            for (auto& pair : sorted_root) {
                if (pair.second == best_move) pair.first = 100000; 
                else pair.first = score_move(pair.second, board, 0, MOVE_NONE);
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