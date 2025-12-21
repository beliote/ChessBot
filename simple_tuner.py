import math

# =========================================================
# CONFIGURATION
# =========================================================
DATA_FILE = "data_stockfish.txt"
LEARNING_RATE = 10.0  # Vitesse d'apprentissage (plus grand = changements plus rapides)
EPOCHS = 500          # Nombre de passages sur les données
K = 2.0               # Facteur de scaling (standard échecs)

# Valeurs initiales (Standard PeSTO ou proches)
# Ce sont les valeurs que l'on va essayer d'améliorer
weights = {
    'P': 100.0,   # Pion
    'N': 320.0,   # Cavalier
    'B': 330.0,   # Fou
    'R': 500.0,   # Tour
    'Q': 900.0    # Dame
}
# =========================================================

def sigmoid(score):
    """Convertit un score en centipawns en probabilité de victoire (0 à 1)"""
    # Protection contre les nombres trop grands (overflow)
    if score > 4000: return 1.0
    if score < -4000: return 0.0
    try:
        return 1 / (1 + 10 ** (-K * score / 400.0))
    except OverflowError:
        return 0.0 if score < 0 else 1.0

def get_material_balance(fen):
    """
    Calcule la différence de matériel du point de vue des BLANCS.
    Ex: Si Blanc a 1 Cavalier de plus, returns {'N': 1, ...}
    """
    board_part = fen.split(" ")[0]
    counts = {'P': 0, 'N': 0, 'B': 0, 'R': 0, 'Q': 0}
    
    for char in board_part:
        if char == '/': continue
        elif char.isdigit(): continue
        
        # Pièces blanches (Positif)
        if char.isupper():
            if char in counts: counts[char] += 1
        # Pièces noires (Négatif)
        elif char.islower():
            upper = char.upper()
            if upper in counts: counts[upper] -= 1
            
    return counts

def evaluate_position(features, current_weights):
    """Calcule le score statique (Matériel uniquement)"""
    score = 0
    for piece, count in features.items():
        score += count * current_weights[piece]
    return score

def main():
    print(f"--- DEMARRAGE DU TUNING ---")
    print(f"Lecture de {DATA_FILE}...")
    
    dataset = []
    try:
        with open(DATA_FILE, "r") as f:
            for line in f:
                parts = line.strip().split("|")
                if len(parts) < 3: continue
                
                fen = parts[0].strip()
                # On récupère le résultat (1.0 = Blanc gagne, 0.0 = Noir gagne)
                try:
                    result = float(parts[2].strip())
                    features = get_material_balance(fen)
                    dataset.append((features, result))
                except ValueError:
                    continue
    except FileNotFoundError:
        print(f"ERREUR: Le fichier {DATA_FILE} n'existe pas !")
        print("Lancez d'abord 'generate_smart_training.py'")
        return

    if not dataset:
        print("Le fichier de données est vide ou mal formé.")
        return

    print(f"Chargé {len(dataset)} positions.")
    print(f"Valeurs initiales : {weights}")
    
    # Calcul de l'erreur initiale
    initial_error = 0
    for features, result in dataset:
        score = evaluate_position(features, weights)
        pred = sigmoid(score)
        initial_error += (result - pred) ** 2
    print(f"Erreur initiale (MSE) : {initial_error / len(dataset):.5f}")
    print("-" * 30)

    # --- BOUCLE D'OPTIMISATION ---
    for epoch in range(EPOCHS):
        total_error = 0
        # On stocke les gradients (la direction dans laquelle modifier les poids)
        gradients = {k: 0.0 for k in weights}
        
        for features, result in dataset:
            # 1. Évaluation actuelle
            score = evaluate_position(features, weights)
            pred = sigmoid(score)
            
            # 2. Calcul de l'erreur
            error = pred - result
            total_error += error ** 2
            
            # 3. Calcul du gradient (Mathématiques de la régression logistique)
            # Facteur commun de la dérivée
            # dSigmoid = K * ln(10) / 400 * pred * (1-pred)
            dS = (K * 2.3025 / 400.0) * pred * (1.0 - pred)
            
            # Gradient pour chaque pièce = erreur * dérivée * nombre_de_pièces
            common_term = 2 * error * dS
            
            for piece, count in features.items():
                if count != 0:
                    gradients[piece] += common_term * count

        # 4. Mise à jour des poids
        # On divise par len(dataset) pour faire une moyenne
        for piece in weights:
            # On soustrait le gradient (on descend la pente de l'erreur)
            weights[piece] -= (gradients[piece] / len(dataset)) * LEARNING_RATE

        if (epoch + 1) % 100 == 0:
            mse = total_error / len(dataset)
            print(f"Epoch {epoch+1:3d} | Erreur: {mse:.6f}")

    print("-" * 30)
    print("=== RESULTATS DU TUNING ===")
    print("Voici les nouvelles valeurs optimales pour VOTRE moteur :")
    print("")
    
    # On arrondit pour le C++
    print(f"const int PAWN_VALUE   = {int(weights['P'])};")
    print(f"const int KNIGHT_VALUE = {int(weights['N'])};")
    print(f"const int BISHOP_VALUE = {int(weights['B'])};")
    print(f"const int ROOK_VALUE   = {int(weights['R'])};")
    print(f"const int QUEEN_VALUE  = {int(weights['Q'])};")
    print("")
    
    # Petit check de cohérence
    print("Analyse :")
    if weights['B'] > weights['N']:
        print("-> Votre moteur valorise correctement la paire de Fous (Fou > Cavalier).")
    else:
        print("-> Votre moteur préfère les Cavaliers (Jeu fermé/Tactique).")

if __name__ == "__main__":
    main()