import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader
import chess
import numpy as np
import os

# =========================================================
# CONFIGURATION
# =========================================================
DATA_FILE = "dataset_nnue.txt"
OUTPUT_HEADER = "include/nnue_weights.h"
BATCH_SIZE = 1024      # Plus gros batch pour stabiliser
EPOCHS = 20            # Un peu plus d'époques
LEARNING_RATE = 0.005  # Vitesse ajustée pour BCE
HIDDEN_SIZE = 128
DEVICE = "cuda" if torch.cuda.is_available() else "cpu"

print(f">>> Mode: {DEVICE}")

# =========================================================
# 1. MAPPING STRICT
# =========================================================
PIECE_MAP = {
    chess.PAWN: 0,
    chess.KNIGHT: 1,
    chess.BISHOP: 2,
    chess.ROOK: 3,
    chess.QUEEN: 4,
    chess.KING: 5
}

def get_feature_indices(board):
    indices = []
    for sq in range(64):
        piece = board.piece_at(sq)
        if piece:
            ptype = PIECE_MAP[piece.piece_type]
            if piece.color == chess.BLACK:
                ptype += 6
            feature_idx = ptype * 64 + sq
            indices.append(feature_idx)
    return indices

# =========================================================
# 2. DATASET
# =========================================================
class ChessDataset(Dataset):
    def __init__(self, filepath):
        self.samples = []
        print(">>> Chargement des données (patience)...")
        with open(filepath, "r") as f:
            for line in f:
                try:
                    parts = line.strip().split("|")
                    fen = parts[0].strip()
                    # On ne garde que la probabilité (0.0 à 1.0)
                    prob = float(parts[2].strip())
                    self.samples.append((fen, prob))
                except:
                    continue
        print(f">>> {len(self.samples)} positions chargées.")

    def __len__(self):
        return len(self.samples)

    def __getitem__(self, idx):
        fen, prob = self.samples[idx]
        board = chess.Board(fen)
        
        input_vec = torch.zeros(768, dtype=torch.float32)
        indices = get_feature_indices(board)
        input_vec[indices] = 1.0
        
        target = torch.tensor([prob], dtype=torch.float32)
        return input_vec, target

# =========================================================
# 3. MODELE (AMÉLIORÉ)
# =========================================================
class SimpleNNUE(nn.Module):
    def __init__(self):
        super().__init__()
        self.l1 = nn.Linear(768, HIDDEN_SIZE)
        self.relu = nn.ReLU()
        self.l2 = nn.Linear(HIDDEN_SIZE, 1)
        
        # --- INITIALISATION KAIMING (CRITIQUE) ---
        # Permet d'éviter que le réseau "meure" au début
        nn.init.kaiming_normal_(self.l1.weight, mode='fan_in', nonlinearity='relu')
        nn.init.zeros_(self.l1.bias)
        nn.init.xavier_normal_(self.l2.weight)
        nn.init.zeros_(self.l2.bias)

    def forward(self, x):
        out = self.l1(x)
        out = self.relu(out)
        return self.l2(out) # On retourne les Logits (pas de sigmoid ici)

# =========================================================
# 4. EXPORT
# =========================================================
def export_weights_to_h(model, filename):
    print(f">>> Exportation haute précision vers {filename}...")
    l1_w = model.l1.weight.detach().cpu().numpy().flatten()
    l1_b = model.l1.bias.detach().cpu().numpy().flatten()
    l2_w = model.l2.weight.detach().cpu().numpy().flatten()
    l2_b = model.l2.bias.detach().cpu().numpy().flatten()
    
    with open(filename, "w") as f:
        f.write("#pragma once\n\n")
        f.write(f"const int HIDDEN_SIZE = {HIDDEN_SIZE};\n\n")
        
        f.write(f"const float L1_WEIGHTS[] = {{\n")
        for val in l1_w: f.write(f"{val:.9e}, ")
        f.write("\n};\n\n")

        f.write(f"const float L1_BIAS[] = {{\n")
        for val in l1_b: f.write(f"{val:.9e}, ")
        f.write("\n};\n\n")

        f.write(f"const float L2_WEIGHTS[] = {{\n")
        for val in l2_w: f.write(f"{val:.9e}, ")
        f.write("\n};\n\n")

        f.write(f"const float L2_BIAS = {l2_b[0]:.9e};\n")
    print(">>> Exportation terminée.")

# =========================================================
# MAIN
# =========================================================
if __name__ == "__main__":
    dataset = ChessDataset(DATA_FILE)
    loader = DataLoader(dataset, batch_size=BATCH_SIZE, shuffle=True)
    
    model = SimpleNNUE().to(DEVICE)
    optimizer = optim.Adam(model.parameters(), lr=LEARNING_RATE)
    
    # --- CHANGEMENT MAJEUR : BCEWithLogitsLoss ---
    # Beaucoup plus stable pour apprendre des probabilités
    criterion = nn.BCEWithLogitsLoss() 
    
    print(">>> Début de l'entraînement (Version Optimisée)...")
    model.train()
    
    for epoch in range(EPOCHS):
        total_loss = 0
        for X, y in loader:
            X, y = X.to(DEVICE), y.to(DEVICE)
            
            optimizer.zero_grad()
            outputs = model(X) # Logits bruts
            
            # BCE calcule la sigmoid en interne de manière sécurisée
            loss = criterion(outputs, y) 
            
            loss.backward()
            optimizer.step()
            total_loss += loss.item()
            
        avg_loss = total_loss / len(loader)
        # La loss BCE est différente de la MSE (elle est autour de 0.69 au début)
        # Si elle descend sous 0.60, c'est que ça apprend !
        print(f"Epoch {epoch+1}/{EPOCHS} | Loss (BCE): {avg_loss:.6f}")

    torch.save(model.state_dict(), "model.pth")
    export_weights_to_h(model, OUTPUT_HEADER)