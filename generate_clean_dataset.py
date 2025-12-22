import chess
import chess.pgn
import random
import math

# CONFIGURATION
OUTPUT_FILE = "dataset_nnue.txt"
POSITIONS_TO_GENERATE = 150000 # On en génère un peu plus car on va filtrer

# Valeurs Matérielles (Classique)
PIECE_VALUES = {
    chess.PAWN: 100,
    chess.KNIGHT: 320,
    chess.BISHOP: 330,
    chess.ROOK: 500,
    chess.QUEEN: 900,
    chess.KING: 20000
}

# Tables PST Simplifiées (Centrées)
PST_PAWN = [
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
]

def static_eval(board):
    # Echecs et Mats : Score Absolu (Positif = Blanc Gagne, Négatif = Noir Gagne)
    if board.is_checkmate():
        if board.turn == chess.WHITE:
            return -20000 # C'est aux blancs de jouer et ils sont mat -> Les noirs gagnent
        else:
            return 20000  # C'est aux noirs de jouer et ils sont mat -> Les blancs gagnent
            
    if board.is_insufficient_material():
        return 0

    score = 0
    # Matériel + PST
    for sq in range(64):
        piece = board.piece_at(sq)
        if piece:
            val = PIECE_VALUES[piece.piece_type]
            
            pst_val = 0
            if piece.piece_type == chess.PAWN:
                rank = chess.square_rank(sq)
                file = chess.square_file(sq)
                pst_sq = sq
                # Miroir des tables pour les noirs
                if piece.color == chess.BLACK:
                    pst_sq = chess.square(file, 7 - rank)
                pst_val = PST_PAWN[pst_sq]

            if piece.color == chess.WHITE:
                score += val + pst_val
            else:
                score -= (val + pst_val)
    
    # --- CORRECTION CRITIQUE ICI ---
    # On a SUPPRIMÉ la ligne : "if board.turn == chess.BLACK: score = -score"
    # Maintenant, le score est TOUJOURS : (Avantage Blanc) - (Avantage Noir)
    
    return score

def generate():
    print(f">>> Génération INTELLIGENTE de {POSITIONS_TO_GENERATE} positions...")
    
    with open(OUTPUT_FILE, "w") as f:
        board = chess.Board()
        count = 0
        games_played = 0
        
        while count < POSITIONS_TO_GENERATE:
            if board.is_game_over() or board.fullmove_number > 80:
                board.reset()
                games_played += 1
            
            # Coups aléatoires
            moves = list(board.legal_moves)
            if not moves:
                board.reset()
                continue
                
            move = random.choice(moves)
            board.push(move)
            
            # --- LE FILTRE CRITIQUE ---
            # On calcule le score
            cp = static_eval(board)
            
            # Si le score est 0 (Egalité parfaite), on a 90% de chances de NE PAS l'écrire.
            # On veut que le réseau mange du déséquilibre !
            if cp == 0 and random.random() > 0.1:
                continue 

            # Calcul Probabilité (Sigmoid)
            try:
                # Echelle douce pour l'entrainement
                prob = 1 / (1 + math.exp(-0.004 * cp))
            except OverflowError:
                prob = 1.0 if cp > 0 else 0.0

            f.write(f"{board.fen()} | {cp} | {prob:.4f}\n")
            count += 1
            
            if count % 10000 == 0:
                print(f"   {count} positions (dont {games_played} parties jouées)...")

    print(">>> Terminé ! Ce dataset est beaucoup plus riche.")

if __name__ == "__main__":
    generate()