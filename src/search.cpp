#include "search.h"
#include "tt.h"
#include "book.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <chrono>

uint64_t Search::nodes_searched = 0;
int Search::max_depth_reached = 0;

// Time management
std::chrono::steady_clock::time_point Search::start_time;
int Search::allotted_time_ms = 0;
bool Search::stop_flag = false;

// Killer moves
Move Search::killer_moves[MAX_DEPTH][MAX_KILLER_MOVES] = {};

// History heuristic
int Search::history[12][64] = {};

void Search::reset_stats() {
    nodes_searched = 0;
    max_depth_reached = 0;
    stop_flag = false;
    
    // Clear killer moves
    for (int i = 0; i < MAX_DEPTH; i++) {
        for (int j = 0; j < MAX_KILLER_MOVES; j++) {
            killer_moves[i][j] = MOVE_NONE;
        }
    }
    
    // Clear history
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 64; j++) {
            history[i][j] = 0;
        }
    }
}

void Search::set_time_limit(int time_limit_ms) {
    allotted_time_ms = time_limit_ms;
    start_time = std::chrono::steady_clock::now();
    stop_flag = false;
}

bool Search::is_time_up() {
    if (allotted_time_ms <= 0) return false; // No time limit
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
    
    // Stop if we've used more than 80% of allotted time
    if (elapsed > (allotted_time_ms * 80) / 100) {
        stop_flag = true;
        return true;
    }
    return false;
}

void Search::stop_search() {
    stop_flag = true;
}

void Search::update_killer_move(Move move, int depth) {
    // We'll check if it's a capture in the calling function
    
    // Shift existing killer moves
    if (killer_moves[depth][0] != move) {
        killer_moves[depth][1] = killer_moves[depth][0];
        killer_moves[depth][0] = move;
    }
}

bool Search::is_killer_move(Move move, int depth) {
    return (killer_moves[depth][0] == move || killer_moves[depth][1] == move);
}

void Search::update_history(Move move, const Board& board, int depth) {
    Square from = get_from_sq(move);
    Square to = get_to_sq(move);
    Piece piece = board.piece_at(from);
    
    if (piece != NO_PIECE && piece < 12) {
        // Bonus increases with depth (deeper = more significant)
        history[piece][to] += depth * depth;
    }
}

int Search::score_move(Move move, const Board& board, int depth) {
    Square from = get_from_sq(move);
    Square to = get_to_sq(move);
    Piece captured = board.piece_at(to);
    Piece moving = board.piece_at(from);
    
    int score = 0;
    
    // MVV-LVA: Most Valuable Victim - Least Valuable Attacker
    // Higher score = better move (captures first)
    if (captured != NO_PIECE || get_move_type(move) == MOVE_TYPE_EN_PASSANT) {
        int victim_value = (captured != NO_PIECE) ? Evaluation::get_piece_value(captured) : Evaluation::PAWN_VALUE;
        int attacker_value = Evaluation::get_piece_value(moving);
        score = 10000 + victim_value - attacker_value;  // Captures get high priority
    } else {
        // Quiet moves: check killer moves first
        if (is_killer_move(move, depth)) {
            score = 9000;  // Killer moves get high priority (but below captures)
        } else {
            // History heuristic for quiet moves
            if (moving != NO_PIECE && moving < 12) {
                score = 1000 + history[moving][to];
            } else {
                score = 1000;
            }
        }
    }
    
    // Promotion moves are very good
    if (get_move_type(move) == MOVE_TYPE_PROMOTION) {
        score += 5000;
    }
    
    return score;
}

void Search::order_moves(std::vector<Move>& moves, const Board& board, int depth) {
    // Create pairs of (move, score)
    std::vector<std::pair<Move, int>> move_scores;
    for (Move move : moves) {
        move_scores.push_back({move, score_move(move, board, depth)});
    }
    
    // Sort by score (descending)
    std::sort(move_scores.begin(), move_scores.end(),
              [](const std::pair<Move, int>& a, const std::pair<Move, int>& b) {
                  return a.second > b.second;
              });
    
    // Update moves vector
    moves.clear();
    for (const auto& pair : move_scores) {
        moves.push_back(pair.first);
    }
}

int Search::quiescence(Board& board, int alpha, int beta) {
    nodes_searched++;
    
    // Stand pat (evaluate current position)
    int stand_pat = Evaluation::evaluate(board);
    if (stand_pat >= beta) {
        return beta;  // Beta cutoff
    }
    if (stand_pat > alpha) {
        alpha = stand_pat;
    }
    
    // Generate only capture moves
    std::vector<Move> moves;
    MoveGenerator::generate_legal_moves(board, moves);
    
    // Filter to only captures
    std::vector<Move> captures;
    for (Move move : moves) {
        Square to = get_to_sq(move);
        if (board.piece_at(to) != NO_PIECE || get_move_type(move) == MOVE_TYPE_EN_PASSANT) {
            captures.push_back(move);
        }
    }
    
    // Order captures (depth 0 for quiescence)
    order_moves(captures, board, 0);
    
    // Search captures
    for (Move move : captures) {
        Board next_board = board;
        next_board.make_move(move);
        
        int score = -quiescence(next_board, -beta, -alpha);
        
        if (score >= beta) {
            return beta;  // Beta cutoff
        }
        if (score > alpha) {
            alpha = score;
        }
    }
    
    return alpha;
}

int Search::negamax(Board& board, int depth, int alpha, int beta, int ply) {
    nodes_searched++;
    
    // Check time every 2048 nodes
    if ((nodes_searched & 2047) == 0) {
        if (is_time_up()) {
            return 0;  // Return immediately if time is up
        }
    }
    
    if (depth > max_depth_reached) {
        max_depth_reached = depth;
    }
    
    // Probe transposition table
    uint64_t hash_key = board.get_hash();
    int tt_score, tt_depth, tt_flag;
    Move tt_move = MOVE_NONE;
    bool tt_hit = TT::probe(hash_key, tt_score, tt_move, tt_depth, tt_flag, depth, alpha, beta);
    
    if (tt_hit && tt_depth >= depth) {
        // Adjust mate scores by ply (they were stored relative to root, now adjust to current position)
        if (tt_score > 20000) {
            tt_score -= ply;
        } else if (tt_score < -20000) {
            tt_score += ply;
        }
        return tt_score;  // TT cutoff
    }
    
    // Terminal node: quiescence search
    if (depth <= 0) {
        return quiescence(board, alpha, beta);
    }
    
    // Generate moves
    std::vector<Move> moves;
    MoveGenerator::generate_legal_moves(board, moves);
    
    // Checkmate or stalemate
    if (moves.empty()) {
        if (board.is_in_check()) {
            int mate_score = -30000 + (100 - depth);  // Checkmate (deeper = better)
            // Store mate score in TT
            TT::store(hash_key, mate_score, MOVE_NONE, depth, TT_EXACT, ply);
            return mate_score;
        }
        // Stalemate
        TT::store(hash_key, 0, MOVE_NONE, depth, TT_EXACT, ply);
        return 0;
    }
    
    // Order moves: hash move first (if available)
    Move hash_move = tt_move;
    if (hash_move != MOVE_NONE) {
        // Move hash move to front if it exists in the move list
        auto it = std::find(moves.begin(), moves.end(), hash_move);
        if (it != moves.end()) {
            moves.erase(it);
            moves.insert(moves.begin(), hash_move);
        }
    }
    
    // Order remaining moves for better pruning
    order_moves(moves, board, depth);
    
    int best_score = -30000;
    Move best_move = MOVE_NONE;
    bool found_pv = false;  // Principal variation move
    int original_alpha = alpha;
    
    for (Move move : moves) {
        // Check time before each move
        if (stop_flag) {
            return 0;
        }
        
        Board next_board = board;
        next_board.make_move(move);
        
        int score;
        
        // Principal Variation Search (PVS): full window for first move, null window for others
        if (!found_pv) {
            // First move: full window
            score = -negamax(next_board, depth - 1, -beta, -alpha, ply + 1);
        } else {
            // Other moves: null window search first
            score = -negamax(next_board, depth - 1, -alpha - 1, -alpha, ply + 1);
            
            // If it fails high, do a full re-search
            if (score > alpha && score < beta) {
                score = -negamax(next_board, depth - 1, -beta, -alpha, ply + 1);
            }
        }
        
        if (stop_flag) {
            return 0;
        }
        
        if (score >= beta) {
            // Beta cutoff: this is a killer move (if it's a quiet move)
            Square to = get_to_sq(move);
            Piece captured = board.piece_at(to);
            if (captured == NO_PIECE && get_move_type(move) != MOVE_TYPE_EN_PASSANT) {
                update_killer_move(move, depth);
            }
            return beta;
        }
        
        if (score > best_score) {
            best_score = score;
            best_move = move;
        }
        
        if (score > alpha) {
            alpha = score;
            found_pv = true;
            best_move = move;
            
            // Update history heuristic for quiet moves that improve alpha
            Square to = get_to_sq(move);
            Piece captured = board.piece_at(to);
            if (captured == NO_PIECE && get_move_type(move) != MOVE_TYPE_EN_PASSANT) {
                update_history(move, board, depth);
            }
        }
    }
    
    // Store result in transposition table
    if (best_score <= original_alpha) {
        tt_flag = TT_ALPHA;  // Upper bound
    } else if (best_score >= beta) {
        tt_flag = TT_BETA;   // Lower bound
    } else {
        tt_flag = TT_EXACT;  // Exact score
    }
    
    TT::store(hash_key, best_score, best_move, depth, tt_flag, ply);
    
    return best_score;
}

Move Search::get_best_move(Board& board, int max_depth, int time_limit_ms) {
    // 1. Vérifier le livre d'ouverture
    Move book_move = Book::get_book_move(board);
    if (book_move != MOVE_NONE) {
        std::cout << "info string Book Move Played" << std::endl;
        return book_move;
    }

    reset_stats();
    set_time_limit(time_limit_ms);
    
    std::vector<Move> moves;
    MoveGenerator::generate_legal_moves(board, moves);
    
    if (moves.empty()) {
        return MOVE_NONE;
    }
    
    Move best_move = moves[0];
    int best_score = -30000;
    
    // Iterative deepening: start at depth 1 and go deeper
    for (int depth = 1; depth <= max_depth && depth <= MAX_DEPTH; depth++) {
        if (stop_flag) {
            // Time is up, return best move from previous completed depth
            break;
        }
        
        int alpha = -30000;
        int beta = 30000;
        Move current_best_move = moves[0];
        int current_best_score = -30000;
        
        // Order moves at root for this iteration
        order_moves(moves, board, 0);
        
        // Search each move at current depth
        for (Move move : moves) {
            if (stop_flag) {
                break;  // Time is up during move search
            }
            
            Board next_board = board;
            next_board.make_move(move);
            
            int score = -negamax(next_board, depth - 1, -beta, -alpha, 1);  // ply=1 at root
            
            if (stop_flag) {
                break;  // Time is up during search
            }
            
            if (score > current_best_score) {
                current_best_score = score;
                current_best_move = move;
            }
            if (score > alpha) {
                alpha = score;
            }
        }
        
        // Only update best_move if we completed the depth without timing out
        if (!stop_flag) {
            best_move = current_best_move;
            best_score = current_best_score;
            
            // Print search info for completed depth
            std::cout << "info depth " << depth 
                      << " score cp " << best_score
                      << " nodes " << nodes_searched;
            
            // Print principal variation (simplified - just best move)
            Square from = get_from_sq(best_move);
            Square to = get_to_sq(best_move);
            char f1 = 'a' + file_of(from);
            char r1 = '8' - rank_of(from);
            char f2 = 'a' + file_of(to);
            char r2 = '8' - rank_of(to);
            std::cout << " pv " << f1 << r1 << f2 << r2;
            
            std::cout << std::endl;
        } else {
            // Time ran out, break and use previous depth's result
            break;
        }
    }
    
    return best_move;
}

