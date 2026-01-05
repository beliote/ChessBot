#pragma once
#include "board.h"
#include "movegenerator.h"
#include "eval.h"
#include <chrono>

class Search {
public:
    // Fonction principale appelée par UCI
    static Move get_best_move(Board& board, int max_depth, int time_limit_ms);

    static void stop_search();
    static void set_time_limit(int time_limit_ms);
    static void reset_stats();

    // Statistiques
    static uint64_t nodes_searched;
    static int max_depth_reached;

    // --- ACCESSIBLES POUR SELFPLAY ---
    // On ajoute '= 0' pour que selfplay.cpp fonctionne sans avoir besoin de fournir 'ply'
    static int negamax(Board& board, int depth, int alpha, int beta, int ply = 0);
    static int quiescence(Board& board, int alpha, int beta, int ply = 0);

private:
    // Heuristiques de tri
    static int score_move(Move move, const Board& board, int ply, Move tt_move);

    // Gestion du temps
    static std::chrono::steady_clock::time_point start_time;
    static int allotted_time_ms;
    static bool stop_flag;
    static bool is_time_up();

    // Tables pour le tri des coups
    static Move killer_moves[64][2];
    static int history[12][64];
};