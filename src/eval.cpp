#include "../include/eval.h"
#include "../include/bitboard.h"
#include <algorithm>

// On garde la déclaration externe pour le NNUE si vous voulez le mixer plus tard
// Pour l'instant, on va se baser sur la solidité du code classique pour corriger les ouvertures
extern int evaluate_nnue(const Board& board);

// ==========================================
// 1. DÉFINITION DES TABLES (PESTO OPTIMISÉ)
// ==========================================

// Valeurs Matérielles (MG = Milieu, EG = Finale)
// On donne un peu plus de valeur aux Fous/Cavaliers en finale pour encourager les échanges si on gagne
const int MG_VAL[6] = { 82, 337, 365, 477, 1025, 0 };
const int EG_VAL[6] = { 94, 281, 297, 512,  936, 0 };

// TABLES MILIEU DE JEU (Middle Game) - Priorité : Sécurité & Activité
const int MG_PAWN[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
     98, 134,  61,  95,  68, 126,  34, -11,
     -6,   7,  26,  31,  65,  56,  25, -20,
    -14,  13,   6,  21,  23,  12,  17, -23,
    -27,  -2,  -5,  12,  17,   6,  10, -25,
    -26,  -4,  -4, -10,   3,   3,  33, -12,
    -35,  -1, -20, -23, -15,  24,  38, -22,
      0,   0,   0,   0,   0,   0,   0,   0
};

const int MG_KNIGHT[64] = {
    -167, -89, -34, -49,  61, -97, -15, -107,
     -73, -41,  72,  36,  23,  62,   7,  -17,
     -47,  60,  37,  65,  84, 129,  73,   44,
      -9,  17,  19,  53,  37,  69,  18,   22,
     -13,   4,  16,  13,  28,  19,  21,   -8,
     -23,  -9,  12,  10,  19,  17,  25,  -16,
     -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23
};

const int MG_BISHOP[64] = {
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21
};

const int MG_ROOK[64] = {
     32,  42,  32,  51,  63,   9,  31,  43,
     27,  32,  58,  62,  80,  67,  26,  44,
     -5,  19,  26,  36,  17,  45,  61,  16,
    -24, -11,   7,  26,  24,  35,  -8, -20,
    -36, -26, -12,  -1,   9,  -7,   6, -23,
    -45, -25, -16, -17,   3,   0,  -5, -33,
    -44, -16, -20,  -9,  -1,  11,  -6, -71,
    -19, -13,   1,  17,  16,   7, -37, -26
};

const int MG_QUEEN[64] = {
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26, -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50
};

const int MG_KING[64] = {
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14
};

// TABLES FINALE (End Game) - Priorité : Centralisation & Pions Passés
const int EG_PAWN[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
     94, 100,  85,  67,  56,  53,  82,  84,
     32,  24,  13,   5,  -2,   4,  17,  17,
     13,   9,  -3,  -7,  -7,  -8,   3,  -1,
      4,   7,  -6,   1,   0,  -5,  -1,  -8,
     13,   8,   8,  10,  13,   0,   2,  -7,
      0,   0,   0,   0,   0,   0,   0,   0
};

const int EG_KNIGHT[64] = {
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64
};

const int EG_BISHOP[64] = {
    -14, -21, -11,  -8,  -7,  -9, -17, -24,
     -8,  -4,   7, -12,  -3, -13,  -4, -14,
      2,  -8,   0,  -1, -13,   3,   0,  -9,
     -3,   9,  12,   4,  14,  10,   3,   2,
     -6,   3,  13,  19,   7,  10,  -3,  -9,
    -12,  -3,   8,  10,  13,   3,  -7, -15,
    -14, -18,  -7,  -1,   4,  -9, -15, -27,
    -23,  -9, -23,  -5,  -9, -16,  -5, -17
};

const int EG_ROOK[64] = {
     13,  10,  18,  15,  12,  12,   8,   5,
     11,  13,  13,  11,  -3,   3,   8,   3,
      7,   7,   7,   5,   4,  -3,  -5,  -3,
      4,   3,  13,   1,   2,   1,  -1,   2,
      3,   5,   8,   4,  -5,  -6,  -8, -11,
     -4,   0,  -5,  -1,  -7, -12,  -8, -16,
     -6,  -6,   0,   2,  -9,  -9, -11,  -3,
     -9,   2,   3,  -1,  -5, -13,   4, -20
};

const int EG_QUEEN[64] = {
    -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
      3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41
};

const int EG_KING[64] = {
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
     10,  17,  23,  15,  20,  45,  44,  13,
     -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  21,  23,  12, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
};

// Tableaux de pointeurs pour simplifier le code
const int* MG_TABLES[6] = { MG_PAWN, MG_KNIGHT, MG_BISHOP, MG_ROOK, MG_QUEEN, MG_KING };
const int* EG_TABLES[6] = { EG_PAWN, EG_KNIGHT, EG_BISHOP, EG_ROOK, EG_QUEEN, EG_KING };

// Poids de "Phase" (pour savoir si on est en finale)
// 0=Pion/Roi (ne comptent pas), 1=Cav/Fou, 2=Tour, 4=Dame
const int GAME_PHASE_INC[6] = { 0, 1, 1, 2, 4, 0 };
// Score Total possible de phase (4 Cav + 4 Fous + 4 Tours + 2 Dames) = 24
const int MAX_PHASE = 24; 

// ==========================================
// 2. LOGIQUE D'ÉVALUATION (HYBRIDE & TAPERED)
// ==========================================

int Evaluation::get_piece_value(Piece piece) {
    return MG_VAL[piece % 6]; // Valeur de base simple
}

int Evaluation::get_pst_value(Piece piece, Square sq) {
    // Cette fonction sert juste de helper externe si besoin
    int type = piece % 6;
    int idx = (piece < 6) ? sq : (sq ^ 56); // Miroir pour les noirs
    return MG_TABLES[type][idx];
}

int Evaluation::evaluate(const Board& board) {
    // 1. Appel du Réseau de Neurones (Tactique pure)
    int nnue_score = evaluate_nnue(board);
    
    // 2. Calcul du "Tapered Eval" (Stratégie Classique)
    int mg[2] = {0, 0}; // Score Milieu de jeu (Blanc, Noir)
    int eg[2] = {0, 0}; // Score Finale (Blanc, Noir)
    int phase = 0;

    for (int p = 0; p < 12; ++p) {
        Bitboard bb = board.pieces[p];
        int type = p % 6;
        int color = p / 6; // 0=Blanc, 1=Noir

        while (bb) {
            Square sq = Bitboards::pop_lsb(bb);
            
            // Calcul de la phase (pour savoir si on est en finale)
            phase += GAME_PHASE_INC[type];

            // Matériel
            mg[color] += MG_VAL[type];
            eg[color] += EG_VAL[type];

            // Position (PST)
            // Pour les blancs : index normal. Pour les noirs : miroir (sq ^ 56)
            int table_idx = (color == WHITE) ? sq : (sq ^ 56);
            
            mg[color] += MG_TABLES[type][table_idx];
            eg[color] += EG_TABLES[type][table_idx];
        }
    }

    // 3. Interpolation de la Phase
    // Phase = 24 (Début) -> on utilise 100% MG
    // Phase = 0 (Rois seuls) -> on utilise 100% EG
    if (phase > MAX_PHASE) phase = MAX_PHASE; // Sécurité
    int mg_score = mg[WHITE] - mg[BLACK];
    int eg_score = eg[WHITE] - eg[BLACK];
    
    // Formule : (MG * phase + EG * (24 - phase)) / 24
    int classic_score = (mg_score * phase + eg_score * (MAX_PHASE - phase)) / MAX_PHASE;

    // 4. Perspective (Blanc ou Noir ?)
    if (board.side_to_move == BLACK) {
        classic_score = -classic_score;
    }

    // 5. Fusion : NNUE + Classique
    // On mélange les deux pour avoir le meilleur des deux mondes.
    // Le NNUE voit les tactiques, le Classique (PST) force le bon placement.
    // On donne un peu plus de poids au NNUE car il est plus précis tactiquement.
    
    return (nnue_score * 3 + classic_score) / 4;
}