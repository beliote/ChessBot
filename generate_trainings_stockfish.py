import chess
import chess.engine
import chess.pgn
import random
import datetime
import sys

# ==================================================================
# CONFIGURATION WINDOWS
# ==================================================================
MY_ENGINE_PATH = r".\build\chess_engine.exe"
STOCKFISH_PATH = r".\stockfish.exe"

DATA_FILE = "data_stockfish.txt"
PGN_FILE = "games.pgn"

# --- CHANGEMENTS ICI ---
GAMES_TO_PLAY = 64     # On réduit le nombre de parties
MOVE_TIME = 0.1        # 0.1s par coup (au lieu de 0.01s). 
                       # Cela permet d'atteindre une profondeur ~5-7.
OPENING_MOVES = 4      # 2 coups complets chacun
TOP_K_MOVES = 10
# ==================================================================

def play_game(white_engine, black_engine, white_name, black_name, data_handle, pgn_handle, round_id):
    board = chess.Board()
    
    # Création de l'objet PGN
    game = chess.pgn.Game()
    game.headers["Event"] = "Training Session (High Quality)"
    game.headers["Site"] = "Local Windows"
    game.headers["Date"] = datetime.datetime.now().strftime("%Y.%m.%d")
    game.headers["Round"] = str(round_id)
    game.headers["White"] = white_name
    game.headers["Black"] = black_name
    
    node = game

    # --- PHASE 1 : OUVERTURE INTELLIGENTE ---
    # On utilise l'un des moteurs pour guider l'ouverture
    opening_guide = black_engine if "Stockfish" in black_name else white_engine

    for _ in range(OPENING_MOVES):
        try:
            # On laisse un peu plus de temps pour l'ouverture aussi
            info = opening_guide.analyse(board, chess.engine.Limit(time=0.2), multipv=TOP_K_MOVES)
            suggested_moves = [entry["pv"][0] for entry in info if "pv" in entry]
            
            if not suggested_moves:
                break
            
            move = random.choice(suggested_moves)
            board.push(move)
            node = node.add_variation(move)
        except Exception:
            if list(board.legal_moves):
                move = random.choice(list(board.legal_moves))
                board.push(move)
                node = node.add_variation(move)

    # --- PHASE 2 : LA PARTIE ---
    training_data = []

    while not board.is_game_over():
        # Qui joue ?
        if board.turn == chess.WHITE:
            current_engine = white_engine
        else:
            current_engine = black_engine

        # On joue le coup avec plus de temps !
        try:
            # --- MODIFICATION ICI : On utilise MOVE_TIME (0.1s) ---
            result = current_engine.play(board, chess.engine.Limit(time=MOVE_TIME))
        except chess.engine.EngineTerminatedError:
            print("Moteur planté !")
            break
        
        if result.move is None:
            break
            
        training_data.append((board.fen(), board.turn))
        
        board.push(result.move)
        node = node.add_variation(result.move)

    # --- FIN DE PARTIE ---
    outcome = board.outcome()
    result_str = "1/2-1/2"
    final_score = 0.5
    
    if outcome and outcome.winner == chess.WHITE:
        result_str = "1-0"
        final_score = 1.0
    elif outcome and outcome.winner == chess.BLACK:
        result_str = "0-1"
        final_score = 0.0

    game.headers["Result"] = result_str
    
    print(game, file=pgn_handle, end="\n\n")
    
    for fen, turn in training_data:
        data_handle.write(f"{fen} | 0 | {final_score}\n")
        
    return result_str

def main():
    print("--- GENERATEUR 64 PARTIES (QUALITE ++ ) ---")
    print(f"MyBot: {MY_ENGINE_PATH}")
    print(f"Stockfish: {STOCKFISH_PATH}")
    print(f"Temps par coup: {MOVE_TIME} sec")
    
    try:
        student = chess.engine.SimpleEngine.popen_uci(MY_ENGINE_PATH)
        teacher = chess.engine.SimpleEngine.popen_uci(STOCKFISH_PATH)
        teacher.configure({"Skill Level": 5}) 
    except FileNotFoundError as e:
        print(f"\nERREUR: {e}")
        return
    except Exception as e:
        print(f"Erreur: {e}")
        return

    with open(DATA_FILE, "a") as f_data, open(PGN_FILE, "a") as f_pgn:
        
        for i in range(GAMES_TO_PLAY):
            if i % 2 == 0:
                res = play_game(student, teacher, "MyBot", "Stockfish", f_data, f_pgn, i+1)
                print(f"Partie {i+1}/{GAMES_TO_PLAY}: MyBot (B) vs Stockfish -> {res}")
            else:
                res = play_game(teacher, student, "Stockfish", "MyBot", f_data, f_pgn, i+1)
                print(f"Partie {i+1}/{GAMES_TO_PLAY}: Stockfish (B) vs MyBot -> {res}")

    student.quit()
    teacher.quit()
    print("\nTerminé !")

if __name__ == "__main__":
    main()