import chess.pgn
import collections
import io

# --- 1. LES DONNÉES PGN (Intégrées directement ici) ---
# Je colle ici le contenu exact que vous m'avez donné.
pgn_content = """
[Event "[Sicilienne] - 4 cavaliers e6: 10.Bxf6"]
[Date "2023.08.25"]
[Result "*"]
[Variant "Standard"]
[ECO "B33"]
[Opening "Sicilian Defense: Lasker-Pelikan Variation, Sveshnikov Variation"]
[FEN "r1bqkb1r/5ppp/p1np1B2/1p2p3/4P3/N3N3/PPP2PPP/R2QKB1R b KQkq - 0 9"]
[SetUp "1"]

9... gxf6 10. Nd5 f5 11. Bd3 (11. c3 Bg7 12. exf5 (12. Bd3 Be6) 12... Bxf5) (11. c4 b4 12. Nc2 fxe4 13. Be2 Bg7 14. Ncxb4 Nd4 15. Nc2 O-O 16. O-O Bb7 17. f3 Bxd5 18. cxd5 f5) (11. Bxb5 axb5 12. Nxb5 Ra4 13. Nbc7+ (13. b4 Rxb4 14. Nbc7+ Kd7 15. O-O Rg8 16. Nxb4 Nxb4 17. Nd5 Nxd5 18. Qxd5 Ke7 19. Rab1 Be6 20. Qb7+ Kf6 21. exf5 Bxf5) 13... Kd7 14. O-O Rxe4 15. Qh5 Nd4 16. c3 (16. Qxf7+ Be7 17. f3 Re2 18. c3 Rf8 19. Qxh7 Ne6 20. Nxe6 Kxe6 21. Rad1 Bb7) 16... Ne2+ 17. Kh1 Kc6 18. g3 Kb7) 11... Be6 12. O-O (12. c3!? Bg7 13. Nxb5 (13. Qh5 O-O 14. O-O (14. exf5 Bxd5 15. f6 e4 16. fxg7 Re8 17. Be2 Ne5 18. O-O Qf6 19. Nc2 Nf3+ 20. Bxf3 Re5 21. Qh3 exf3 22. Ne3 fxg2 23. Rfd1 Bf3 24. Rd4 Rae8) 14... f4 15. Rfd1 (15. Nc2 f5 16. a4 (16. Ncb4 Nxb4 17. Nxb4 a5 18. Nc2 (18. exf5 Bf7 19. Qh3 Qf6 20. Nc2 b4 21. cxb4 d5) 18... Qf6 19. exf5 Bf7 20. Qg4 b4 21. cxb4 d5) 16... Bxd5 17. exd5 Ne7 18. axb5 e4 19. Bc4 axb5 20. Bxb5 Rb8 21. Bc6 Rxb2) 15... Kh8 16. Nc2 f5 17. exf5 Bxd5 18. f6 Bh6 19. Qxh6 Ra7) (13. Nc2 Bxd5 14. exd5 Ne7 15. O-O O-O 16. a4 e4 17. Be2 bxa4 18. Rxa4 Qb6 19. Ra2 f4 20. Qd2 Be5 21. Rfa1 Kh8 22. Bf1) 13... axb5 14. Bxb5 Rc8 15. Qa4 Bd7 16. exf5 O-O 17. O-O e4 18. Qxe4 (18. Rfe1 Ne5 19. Bxd7 Nxd7 20. Qxe4 Nf6 21. Nxf6+ Bxf6 22. Rad1 Qb6 23. Qg4+ Kh8 24. Re2 d5 25. g3 (25. Rxd5) 25... Qb5 26. Red2 d4 27. cxd4 Qd5) 18... Re8 19. Qa4 (19. Qf4 Re5 20. Bxc6 Bxc6 21. Ne3) 19... Re5 20. Rad1 Rxf5 21. Rfe1 Kh8 22. f4 Rh5 23. h3 Rh6 24. Re2 Re6 25. Rde1 Nb8 26. Bxd7 Nxd7) 12... Bxd5 13. exd5 Ne7 14. c3 (14. Nxb5!? Bg7 15. Nc3 e4 16. Bc4 (16. Be2 Ng6 17. Qd2 O-O 17... Ng6 17. Qd2 O-O 18. Rab1 Rb8 19. Bxa6 Qa5 20. Bc4 Rxb2 21. Rxb2 Bxc3 22. Qc1 Bxb2 23. Qxb2 Rc8) 16... O-O 17. Qd2 (17. Qh5 Qc8 18. Bb3 Bxc3 19. bxc3 f4) 17... Ng6 18. Kh1 Qc7 19. Bb3 Bxc3 20. bxc3 f4 21. Rab1 Rac8) 14... Bg7 15. Qh5 e4 16. Bc2 (16. Be2 O-O 17. Nc2 Kh8 18. a4 bxa4 19. Rxa4 Rb8 20. Ra2 a5) 16... O-O 17. Rae1 (17. f3 b4 18. Nc4 Rc8 19. b3 Nxd5 20. fxe4 Rxc4 21. bxc4 Ne3 22. Rf2 Qb6 23. Kh1 bxc3) 17... Qc8 18. Kh1 Rb8 *
"""

OUTPUT_FILE = "generated_book.txt"

def clean_fen_key(board):
    fen = board.fen()
    parts = fen.split()
    return " ".join(parts[:3])

def process_node(node, board, book):
    if node.is_end():
        return

    for variation in node.variations:
        move = variation.move
        fen_key = clean_fen_key(board)
        uci_move = move.uci()
        
        book[fen_key].add(uci_move)
        
        board.push(move)
        process_node(variation, board, book)
        board.pop()

def main():
    print("1. Lecture des données PGN intégrées...")
    
    # On utilise io.StringIO pour lire la chaîne comme un fichier
    pgn_file = io.StringIO(pgn_content)
    book = collections.defaultdict(set)
    
    games_count = 0
    while True:
        game = chess.pgn.read_game(pgn_file)
        if game is None:
            break
            
        board = game.board() # Initialise le board avec le FEN du PGN
        games_count += 1
        process_node(game, board, book)

    print(f"   -> {games_count} chapitre(s) traité(s).")
    
    print(f"2. Génération du fichier C++ : {OUTPUT_FILE}...")
    with open(OUTPUT_FILE, "w", encoding="utf-8") as f:
        f.write("// Code genere depuis PGN\n")
        
        count = 0
        for fen, moves_set in book.items():
            moves_list = sorted(list(moves_set))
            moves_str = ", ".join([f'"{m}"' for m in moves_list])
            
            line = f'    book_moves["{fen}"] = {{{moves_str}}};\n'
            f.write(line)
            count += 1

    print(f"Succès ! {count} positions ajoutées.")

if __name__ == "__main__":
    main()