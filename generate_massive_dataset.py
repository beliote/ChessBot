import chess
import chess.engine
import chess.pgn
import random
import datetime
import time
import sys

# ==================================================================
# CONFIGURATION "USINE"
# ==================================================================
MY_ENGINE_PATH = r".\build\chess_engine.exe"
STOCKFISH_PATH = r".\stockfish.exe"

DATA_FILE = "dataset_nnue.txt"  
PGN_FILE = "massive_games.pgn"  

GAMES_TO_PLAY = 5000   
MOVE_TIME = 0.05       
                       
OPENING_MOVES = 8      
TOP_K_MOVES = 15       
# ==================================================================

def play_game(student, teacher, round_id, f_data, f_pgn):
    board = chess.Board()
    
    # --- CORRECTION ICI ---
    # On garde une référence fixe vers la RACINE pour les headers
    root_game = chess.pgn.Game() 
    root_game.headers["Event"] = "Massive Data Gen"
    root_game.headers["Round"] = str(round_id)
    root_game.headers["Date"] = datetime.datetime.now().strftime("%Y.%m.%d")
    
    # On utilise 'node' pour naviguer dans l'arbre des coups
    node = root_game 
    
    # --- 1. OUVERTURE ASSISTÉE (Rapide) ---
    for _ in range(OPENING_MOVES):
        try:
            limit = chess.engine.Limit(time=0.01)
            info = teacher.analyse(board, limit, multipv=TOP_K_MOVES)
            candidates = [entry["pv"][0] for entry in info if "pv" in entry]
            
            if not candidates:
                if not list(board.legal_moves): break
                candidates = list(board.legal_moves)
            
            move = random.choice(candidates)
            board.push(move)
            node = node.add_variation(move)
        except:
            break

    # --- 2. LA PARTIE ---
    if round_id % 2 == 0:
        white_engine, black_engine = student, teacher
        # On écrit sur root_game, pas sur node !
        root_game.headers["White"], root_game.headers["Black"] = "MyBot", "Stockfish"
    else:
        white_engine, black_engine = teacher, student
        root_game.headers["White"], root_game.headers["Black"] = "Stockfish", "MyBot"

    training_data = []

    while not board.is_game_over():
        current_engine = white_engine if board.turn == chess.WHITE else black_engine
        
        try:
            result = current_engine.play(board, chess.engine.Limit(time=MOVE_TIME))
        except chess.engine.EngineTerminatedError:
            return None 
            
        if result.move is None:
            break
            
        training_data.append((board.fen(), board.turn))
        
        board.push(result.move)
        node = node.add_variation(result.move)
        
        if len(training_data) > 300:
            break

    # --- 3. RESULTAT ---
    outcome = board.outcome()
    final_score = 0.5 
    result_str = "1/2-1/2"
    
    if outcome:
        if outcome.winner == chess.WHITE: 
            final_score = 1.0
            result_str = "1-0"
        elif outcome.winner == chess.BLACK: 
            final_score = 0.0
            result_str = "0-1"
            
    root_game.headers["Result"] = result_str

    # Optionnel : Ecriture PGN
    # print(root_game, file=f_pgn, end="\n\n") 
    
    # Ecriture Dataset
    for fen, turn in training_data:
        f_data.write(f"{fen} | 0 | {final_score}\n")
        
    return 1 

def main():
    print(f"--- GENERATION MASSIVE ({GAMES_TO_PLAY} parties) ---")
    print(f"Sortie: {DATA_FILE}")
    print("Appuyez sur Ctrl+C pour arrêter proprement.\n")
    
    try:
        student = chess.engine.SimpleEngine.popen_uci(MY_ENGINE_PATH)
        teacher = chess.engine.SimpleEngine.popen_uci(STOCKFISH_PATH)
        teacher.configure({"Skill Level": 7}) 
    except Exception as e:
        print(f"Erreur Moteurs: {e}")
        return

    start_time = time.time()
    games_played = 0
    
    with open(DATA_FILE, "a") as f_data, open(PGN_FILE, "a") as f_pgn:
        try:
            for i in range(GAMES_TO_PLAY):
                res = play_game(student, teacher, i+1, f_data, f_pgn)
                
                if res:
                    games_played += 1
                
                elapsed = time.time() - start_time
                avg_time = elapsed / (i + 1)
                remaining = (GAMES_TO_PLAY - (i + 1)) * avg_time
                
                sys.stdout.write(f"\rParties: {i+1}/{GAMES_TO_PLAY} | "
                                 f"Temps écoulé: {int(elapsed)}s | "
                                 f"Restant env: {int(remaining/60)}min ")
                sys.stdout.flush()
                
        except KeyboardInterrupt:
            print("\n\nArrêt utilisateur demandé.")
        except Exception as e:
            print(f"\n\nErreur inattendue: {e}")

    student.quit()
    teacher.quit()
    print(f"\n\nTerminé ! {games_played} parties générées dans {DATA_FILE}.")

if __name__ == "__main__":
    main()