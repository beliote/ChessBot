#include <algorithm>
#include <cmath>
#include <vector>
#include "../include/board.h"
#include "../include/nnue_weights.h"
#include "../include/types.h"

// Configuration
constexpr int INPUT_SIZE = 768; 
constexpr float SCORE_SCALE = 400.0f;

// Mapping direct (Pawn=0 ... King=5, Black offset +6)
const int PIECE_MAP[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

int evaluate_nnue(const Board& board) {
    // OPTIMISATION 1 : Tableau brut aligné sur la pile (plus rapide que std::vector)
    alignas(64) float hidden_layer[HIDDEN_SIZE];
    
    // 1. Copie des biais
    std::copy(std::begin(L1_BIAS), std::end(L1_BIAS), hidden_layer);

    // 2. Accumulation
    for (int sq = 0; sq < 64; ++sq) {
        int piece = board.piece_at(sq);
        
        if (piece != NO_PIECE && piece >= 0 && piece <= 11) {
            // Miroir Vertical A8->A1
            int nnue_sq = sq ^ 56; 
            int input_idx = PIECE_MAP[piece] * 64 + nnue_sq;
            
            // Boucle critique (Vectorisée par le compilateur)
            for (int i = 0; i < HIDDEN_SIZE; ++i) {
                hidden_layer[i] += L1_WEIGHTS[i * INPUT_SIZE + input_idx];
            }
        }
    }

    // 3. Activation (ReLU) & Sortie
    float output = L2_BIAS;
    for (int i = 0; i < HIDDEN_SIZE; ++i) {
        if (hidden_layer[i] > 0.0f) {
            output += hidden_layer[i] * L2_WEIGHTS[i];
        }
    }

    // OPTIMISATION 2 : Suppression de std::exp
    // On utilise une approximation linéaire directe du logit.
    // C'est beaucoup plus rapide et préserve l'ordre des coups.
    int score_cp = static_cast<int>(output * 100.0f);

    // 4. Relatif au trait
    if (board.side_to_move == BLACK) {
        score_cp = -score_cp;
    }

    return score_cp;
}