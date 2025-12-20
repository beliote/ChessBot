# CppChess Engine

A high-performance chess engine written in C++17, featuring bitboard representation, magic bitboard sliding piece attacks, alpha-beta pruning with quiescence search, and comprehensive move generation validated through perft testing.

## Features

- **Bitboard Representation**: Efficient 64-bit board representation for fast move generation
- **Magic Bitboards**: Pre-calculated attack tables for sliding pieces (bishops and rooks)
- **Move Generation**: Complete legal move generation with support for:
  - All piece types (pawns, knights, bishops, rooks, queens, kings)
  - Special moves (castling, en passant, promotions)
  - Move validation and legality checking
- **Search Algorithm**: 
  - Alpha-beta pruning for efficient search
  - Quiescence search to avoid horizon effect
  - MVV-LVA move ordering for better pruning
- **Evaluation**: 
  - Material evaluation (P=100, N=320, B=330, R=500, Q=900, K=20000)
  - Piece-square tables for positional evaluation
  - Coordinate system: A8=0, H1=63
- **Perft Validated**: Move generation verified through perft testing (depth 5: 4,865,609 nodes)

## Project Structure

```
CppChess Engine/
├── include/           # Header files
│   ├── types.h
│   ├── bitboard.h
│   ├── board.h
│   ├── movegenerator.h
│   ├── eval.h
│   └── search.h
├── src/              # Source files
│   ├── main.cpp
│   ├── bitboard.cpp
│   ├── board.cpp
│   ├── movegenerator.cpp
│   ├── eval.cpp
│   └── search.cpp
├── CMakeLists.txt    # CMake build configuration
├── Makefile          # Makefile for Linux/macOS
├── build.bat         # Build script for Windows
└── README.md          # This file
```

## Building the Project

### Prerequisites

- **C++17 compatible compiler**:
  - Windows: MinGW-w64 (g++), MSVC, or Clang
  - Linux: g++ or clang++ (usually pre-installed)
  - macOS: Xcode Command Line Tools (`xcode-select --install`)

- **CMake** (optional, for Option A):
  - Windows: Download from [cmake.org](https://cmake.org/download/)
  - Linux: `sudo apt-get install cmake` (Ubuntu/Debian) or `sudo yum install cmake` (RHEL/CentOS)
  - macOS: `brew install cmake` or download from cmake.org

### Option A: Build with CMake (Recommended)

CMake provides the most reliable cross-platform build experience.

#### Windows:
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The executable will be in `build/Release/chess_engine.exe` (or `build/chess_engine.exe` depending on generator).

#### Linux/macOS:
```bash
mkdir build
cd build
cmake ..
make
```

The executable will be in `build/chess_engine`.

### Option B: Quick Build on Windows

If you have MinGW or g++ installed:

```bash
build.bat
```

The executable will be in `build/chess_engine.exe`.

**Note**: Make sure `g++` is in your PATH. If using MinGW, add `C:\MinGW\bin` to your system PATH.

### Option C: Quick Build on Linux/macOS

```bash
make
```

The executable will be `chess_engine` in the project root.

To clean build artifacts:
```bash
make clean
```

To rebuild from scratch:
```bash
make rebuild
```

## Usage

After building, run the executable:

### Windows:
```bash
build\chess_engine.exe
# or if using CMake:
build\Release\chess_engine.exe
```

### Linux/macOS:
```bash
./chess_engine
# or if using CMake:
./build/chess_engine
```

The engine will:
1. Initialize the bitboard masks and attack tables
2. Set up a test position (mate in 2 puzzle)
3. Search for the best move at depth 5
4. Display the best move and resulting position
5. Show evaluation scores

## Example Output

```
==========================================
  CHESS ENGINE - AI SEARCH TEST
==========================================

Initial Position:
[Board display]

Static Evaluation: 150 cp

Searching for best move (depth 5)...
------------------------------------------
info depth 5 score cp 250 nodes 125000 pv e2e4
------------------------------------------
Search completed in 1250 ms
Nodes searched: 125000
Max depth reached: 5

Best Move: e2e4

Position after best move:
[Board display]

New Static Evaluation: 250 cp
```

## Technical Details

### Coordinate System
- Square 0 = A8 (top-left)
- Square 63 = H1 (bottom-right)
- White pieces start at indices 48-63 (bottom of board), move UP (index decreases)
- Black pieces start at indices 0-15 (top of board), move DOWN (index increases)

### Move Representation
- 16-bit move encoding:
  - Bits 0-5: from square (0-63)
  - Bits 6-11: to square (0-63)
  - Bits 12-13: promotion piece (0-3: N, B, R, Q)
  - Bits 14-15: move type (0=normal, 1=promotion, 2=en passant, 3=castling)

### Performance
- Optimized with compiler flags (`-O3`, `-march=native`)
- Efficient bitboard operations using compiler intrinsics
- Pre-calculated attack tables for fast move generation

## License

This project is provided as-is for educational and personal use.

## Contributing

Feel free to submit issues, fork the repository, and create pull requests for any improvements.

## Acknowledgments

Built with modern C++ best practices, inspired by chess programming techniques from the chess engine development community.
