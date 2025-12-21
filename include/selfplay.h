#pragma once
#include "board.h"
#include <string>
#include <vector>

struct TrainingPosition {
    std::string fen;
    int score;         // Score vu par le moteur au moment de jouer
    int player_turn;   // WHITE (0) ou BLACK (1)
};

class SelfPlay {
public:
    // Lance la génération de 'num_games' parties
    static void start_training(int num_games, int depth);

private:
    // Joue une seule partie et retourne le résultat (1.0 blanc, 0.0 noir, 0.5 nul)
    // Remplit le vecteur 'positions' avec les données de la partie
    static float play_game(int depth, std::vector<TrainingPosition>& positions);
    
    // Sauvegarde les données dans un fichier texte
    static void save_data(const std::vector<TrainingPosition>& positions, float result);
    
    // Vérifie si la partie est finie (Mat, Nulle, 50 coups...)
    static int check_game_over(Board& board); // Retourne 0:Continue, 1:Blanc gagne, 2:Noir gagne, 3:Nulle
};