#include "selfplay.h"
#include "search.h"
#include "movegenerator.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

// Codes de fin de partie
#define GAME_CONTINUE 0
#define GAME_WHITE_WIN 1
#define GAME_BLACK_WIN 2
#define GAME_DRAW 3

void SelfPlay::start_training(int num_games, int depth) {
    std::srand(std::time(nullptr));
    std::cout << "=== DEMARRAGE DU SELF-PLAY ===" << std::endl;
    std::cout << "Objectif : " << num_games << " parties a profondeur " << depth << std::endl;

    for (int i = 1; i <= num_games; i++) {
        std::vector<TrainingPosition> game_positions;
        float result = play_game(depth, game_positions);
        
        save_data(game_positions, result);
        
        if (i % 10 == 0) {
            std::cout << "Parties jouees : " << i << " / " << num_games << "\r" << std::flush;
        }
    }
    std::cout << "\n=== GENERATION TERMINEE ! ===" << std::endl;
}

float SelfPlay::play_game(int depth, std::vector<TrainingPosition>& positions) {
    Board board;
    // board.set_fen("..."); // Startpos par défaut
    board.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    int moves_count = 0;
    
    while (true) {
        // 1. Vérifier fin de partie (Mat / Nulle)
        int status = check_game_over(board);
        if (status != GAME_CONTINUE) {
            if (status == GAME_WHITE_WIN) return 1.0f;
            if (status == GAME_BLACK_WIN) return 0.0f;
            return 0.5f; // Nulle
        }

        // 2. Choisir un coup
        Move best_move = MOVE_NONE;
        int score = 0;

        // OUVERTURE ALEATOIRE (8 premiers demi-coups = 4 coups complets)
        // Pour varier les parties et explorer différentes positions
        if (moves_count < 8) {
            std::vector<Move> moves;
            MoveGenerator::generate_legal_moves(board, moves);
            if (moves.empty()) break; // Bug safety
            
            int random_idx = std::rand() % moves.size();
            best_move = moves[random_idx];
            score = 0; // Pas de score fiable en random
        } 
        else {
            // RECHERCHE NORMALE
            // On utilise negamax directement pour avoir le score
            // Note: depth est faible pour la vitesse
            score = Search::negamax(board, depth, -50000, 50000);
            
            // On récupère le meilleur coup via une petite recherche (ou TT si stocké)
            // Pour simplifier ici, on refait un get_best_move rapide
            // Idéalement on modifierait search pour retourner coup + score ensemble
            best_move = Search::get_best_move(board, depth, 0); 
        }

        if (best_move == MOVE_NONE) break; // Devrait être géré par check_game_over

        // 3. Stocker la position AVANT de jouer le coup
        // On ne stocke que APRES l'ouverture aléatoire pour apprendre sur des positions "réelles"
        if (moves_count >= 8) {
            TrainingPosition pos;
            pos.fen = board.get_fen();
            pos.score = score;
            pos.player_turn = board.side_to_move;
            positions.push_back(pos);
        }

        // 4. Jouer le coup
        board.make_move(best_move);
        moves_count++;
        
        // Sécurité : arrêt si partie trop longue (ex: 300 coups)
        if (moves_count > 300) return 0.5f;
    }
    
    return 0.5f;
}

void SelfPlay::save_data(const std::vector<TrainingPosition>& positions, float result) {
    std::ofstream file("data.txt", std::ios::app); // Mode 'append'
    if (!file.is_open()) return;

    for (const auto& pos : positions) {
        // Format : FEN | SCORE_MOTEUR | RESULTAT_FINAL
        // Attention : Le résultat final est global (1.0 = Blanc gagne).
        // Mais l'évaluation (pos.score) est relative au joueur qui joue.
        // Pour le Tuning, on aime souvent tout normaliser du point de vue des Blancs,
        // ou garder relatif. Gardons simple : tout relatif aux Blancs.
        
        // Si c'était aux noirs de jouer, le score negamax était du point de vue noir.
        // On l'inverse pour l'avoir toujours du point de vue Blanc pour l'analyse.
        int white_score = (pos.player_turn == WHITE) ? pos.score : -pos.score;
        
        file << pos.fen << " | " << white_score << " | " << result << "\n";
    }
    file.close();
}

int SelfPlay::check_game_over(Board& board) {
    // 1. Répétition ou 50 coups
    if (board.is_repetition() || board.halfmove_clock >= 100) {
        return GAME_DRAW;
    }

    std::vector<Move> moves;
    MoveGenerator::generate_legal_moves(board, moves);

    // 2. Mat ou Pat
    if (moves.empty()) {
        if (board.is_in_check()) {
            return (board.side_to_move == WHITE) ? GAME_BLACK_WIN : GAME_WHITE_WIN;
        }
        return GAME_DRAW; // Pat
    }

    // 3. Matériel insuffisant (Simplifié)
    // Si pas de pions et pas de pièces majeures... (A améliorer plus tard)
    // Pour l'instant on fait confiance à l'horloge des 50 coups.
    
    return GAME_CONTINUE;
}