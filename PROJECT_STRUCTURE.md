# Project Structure Organization

## File Organization

### Headers (→ `include/` folder)
All header files (`.h`) should be moved to the `include/` directory:

- `types.h` → `include/types.h`
- `bitboard.h` → `include/bitboard.h`
- `board.h` → `include/board.h`
- `movegenerator.h` → `include/movegenerator.h`
- `eval.h` → `include/eval.h`
- `search.h` → `include/search.h`

### Sources (→ `src/` folder)
All implementation files (`.cpp`) should be moved to the `src/` directory:

- `main.cpp` → `src/main.cpp`
- `bitboard.cpp` → `src/bitboard.cpp`
- `board.cpp` → `src/board.cpp`
- `movegenerator.cpp` → `src/movegenerator.cpp`
- `eval.cpp` → `src/eval.cpp`
- `search.cpp` → `src/search.cpp`

## Migration Steps

1. Create the directories:
   ```bash
   mkdir include src
   ```

2. Move header files:
   ```bash
   # Windows
   move *.h include\
   
   # Linux/macOS
   mv *.h include/
   ```

3. Move source files:
   ```bash
   # Windows
   move *.cpp src\
   
   # Linux/macOS
   mv *.cpp src/
   ```

4. No changes needed to `#include` statements - they already use relative paths like `#include "board.h"`, which will work with the `-Iinclude` flag.

## Final Structure

```
CppChess Engine/
├── include/
│   ├── types.h
│   ├── bitboard.h
│   ├── board.h
│   ├── movegenerator.h
│   ├── eval.h
│   └── search.h
├── src/
│   ├── main.cpp
│   ├── bitboard.cpp
│   ├── board.cpp
│   ├── movegenerator.cpp
│   ├── eval.cpp
│   └── search.cpp
├── CMakeLists.txt
├── Makefile
├── build.bat
├── README.md
└── .gitignore
```

