import chess
import chess.pgn
import random
import math

# CONFIGURATION
OUTPUT_FILE = "dataset_nnue.txt"
POSITIONS_TO_GENERATE = 200000 # Un peu plus pour bien apprendre

# --- TABLES PeSTO (Middlegame) ---
# Ces valeurs sont issues de millions de parties de moteurs forts.
# Elles guident le positionnement stratégique.

# Tables inversées (A1=0 .. H8=63) pour python-chess
mg_pawn_table = [
      0,   0,   0,   0,   0,   0,   0,   0,
     98, 134,  61,  95,  68, 126,  34, -11,
     -6,   7,  26,  31,  65,  56,  25, -20,
    -14,  13,   6,  21,  23,  12,  17, -23,
    -27,  -2,  -5,  12,  17,   6,  10, -25,
    -26,  -4,  -4, -10,   3,   3,  33, -12,
    -35,  -1, -20, -23, -15,  24,  38, -22,
      0,   0,   0,   0,   0,   0,   0,   0,
]

mg_knight_table = [
    -167, -89, -34, -49,  61, -97, -15, -107,
     -73, -41,  72,  36,  23,  62,   7,  -17,
     -47,  60,  37,  65,  84, 129,  73,   44,
      -9,  17,  19,  53,  37,  69,  18,   22,
     -13,   4,  16,  13,  28,  19,  21,   -8,
     -23,  -9,  12,  10,  19,  17,  25,  -16,
     -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23,
]

mg_bishop_table = [
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,
]

mg_rook_table = [
     32,  42,  32,  51,  63,   9,  31,  43,
     27,  32,  58,  62,  80,  67,  26,  44,
     -5,  19,  26,  36,  17,  45,  61,  16,
    -24, -11,   7,  26,  24,  35,  -8, -20,
    -36, -26, -12,  -1,   9,  -7,   6, -23,
    -45, -25, -16, -17,   3,   0,  -5, -33,
    -44, -16, -20,  -9,  -1,  11,  -6, -71,
    -19, -13,   1,  17,  16,   7, -37, -26,
]

mg_queen_table = [
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26, -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50,
]

mg_king_table = [
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,
]

# Valeurs de base (Centipawns)
MG_VALUE = {
    chess.PAWN: 82,
    chess.KNIGHT: 337,
    chess.BISHOP: 365,
    chess.ROOK: 477,
    chess.QUEEN: 1025,
    chess.KING: 0
}

def get_pesto_score(board):
    if board.is_checkmate():
        if board.turn == chess.WHITE: return -20000
        else: return 20000
        
    score = 0
    
    # Matériel + Positionnel
    for sq in range(64):
        piece = board.piece_at(sq)
        if piece:
            ptype = piece.piece_type
            color = piece.color
            
            # Valeur de base
            val = MG_VALUE[ptype]
            
            # Bonus Positionnel (Table)
            # Pour les blancs, index direct. Pour les noirs, miroir vertical.
            idx = sq if color == chess.WHITE else chess.square_mirror(sq)
            
            pst_val = 0
            if ptype == chess.PAWN: pst_val = mg_pawn_table[idx]
            elif ptype == chess.KNIGHT: pst_val = mg_knight_table[idx]
            elif ptype == chess.BISHOP: pst_val = mg_bishop_table[idx]
            elif ptype == chess.ROOK: pst_val = mg_rook_table[idx]
            elif ptype == chess.QUEEN: pst_val = mg_queen_table[idx]
            elif ptype == chess.KING: pst_val = mg_king_table[idx]
            
            if color == chess.WHITE:
                score += val + pst_val
            else:
                score -= (val + pst_val)
                
    return score

def generate():
    print(f">>> Génération PeSTO de {POSITIONS_TO_GENERATE} positions...")
    
    with open(OUTPUT_FILE, "w") as f:
        board = chess.Board()
        count = 0
        
        while count < POSITIONS_TO_GENERATE:
            if board.is_game_over() or board.fullmove_number > 80:
                board.reset()
            
            moves = list(board.legal_moves)
            if not moves:
                board.reset()
                continue
                
            # On joue des coups un peu plus "intelligents" que le pur hasard pour avoir des positions réalistes
            # On prend un coup au hasard parmi les captures s'il y en a, sinon hasard
            captures = [m for m in moves if board.is_capture(m)]
            if captures and random.random() > 0.5:
                move = random.choice(captures)
            else:
                move = random.choice(moves)
                
            board.push(move)
            
            # Évaluation PeSTO
            cp = get_pesto_score(board)
            
            # Filtre : On évite les positions nulles parfaites (0)
            if cp == 0 and random.random() > 0.2:
                continue 

            # Conversion Sigmoid pour l'apprentissage
            try:
                prob = 1 / (1 + math.exp(-0.004 * cp))
            except OverflowError:
                prob = 1.0 if cp > 0 else 0.0

            f.write(f"{board.fen()} | {cp} | {prob:.4f}\n")
            count += 1
            
            if count % 10000 == 0:
                print(f"   {count} positions...")

    print(">>> Terminé ! Le dataset contient maintenant de la stratégie.")

if __name__ == "__main__":
    generate()