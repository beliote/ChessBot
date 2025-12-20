#include "search.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

uint64_t Search::nodes_searched = 0;
int Search::max_depth_reached = 0;

void Search::reset_stats() {
    nodes_searched = 0;
    max_depth_reached = 0;
}

int Search::score_move(Move move, const Board& board) {
    Square from = get_from_sq(move);
    Square to = get_to_sq(move);
    Piece captured = board.piece_at(to);
    Piece moving = board.piece_at(from);
    
    int score = 0;
    
    // MVV-LVA: Most Valuable Victim - Least Valuable Attacker
    // Higher score = better move (captures first)
    if (captured != NO_PIECE) {
        int victim_value = Evaluation::get_piece_value(captured);
        int attacker_value = Evaluation::get_piece_value(moving);
        score = 10000 + victim_value - attacker_value;  // Captures get high priority
    } else {
        // Non-captures get lower priority
        score = 1000;
    }
    
    // Promotion moves are very good
    if (get_move_type(move) == MOVE_TYPE_PROMOTION) {
        score += 5000;
    }
    
    return score;
}

void Search::order_moves(std::vector<Move>& moves, const Board& board) {
    // Create pairs of (move, score)
    std::vector<std::pair<Move, int>> move_scores;
    for (Move move : moves) {
        move_scores.push_back({move, score_move(move, board)});
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
    
    // Order captures
    order_moves(captures, board);
    
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

int Search::negamax(Board& board, int depth, int alpha, int beta) {
    nodes_searched++;
    if (depth > max_depth_reached) {
        max_depth_reached = depth;
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
            return -30000 + (100 - depth);  // Checkmate (deeper = better)
        }
        return 0;  // Stalemate
    }
    
    // Order moves for better pruning
    order_moves(moves, board);
    
    int best_score = -30000;
    
    for (Move move : moves) {
        Board next_board = board;
        next_board.make_move(move);
        
        int score = -negamax(next_board, depth - 1, -beta, -alpha);
        
        if (score >= beta) {
            return beta;  // Beta cutoff
        }
        if (score > best_score) {
            best_score = score;
        }
        if (score > alpha) {
            alpha = score;
        }
    }
    
    return best_score;
}

Move Search::get_best_move(Board& board, int depth) {
    reset_stats();
    
    std::vector<Move> moves;
    MoveGenerator::generate_legal_moves(board, moves);
    
    if (moves.empty()) {
        return MOVE_NONE;
    }
    
    Move best_move = moves[0];
    int best_score = -30000;
    int alpha = -30000;
    int beta = 30000;
    
    // Order moves at root
    order_moves(moves, board);
    
    // Search each move
    for (Move move : moves) {
        Board next_board = board;
        next_board.make_move(move);
        
        int score = -negamax(next_board, depth - 1, -beta, -alpha);
        
        if (score > best_score) {
            best_score = score;
            best_move = move;
        }
        if (score > alpha) {
            alpha = score;
        }
    }
    
    // Print search info
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
    
    return best_move;
}

