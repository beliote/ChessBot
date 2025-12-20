# Build Instructions Summary

## Quick Reference

### 1. Project Structure
- **Headers**: All `.h` files → `include/` folder
- **Sources**: All `.cpp` files → `src/` folder

### 2. Build Methods

#### Method A: CMake (Cross-Platform, Recommended)
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

#### Method B: Windows Quick Build
```bash
build.bat
```

#### Method C: Linux/macOS Quick Build
```bash
make
```

## File Organization

Before building, organize your files:

```
include/          src/
├── types.h       ├── main.cpp
├── bitboard.h    ├── bitboard.cpp
├── board.h       ├── board.cpp
├── movegenerator.h ├── movegenerator.cpp
├── eval.h        ├── eval.cpp
└── search.h      └── search.cpp
```

## Verification

After building, verify the executable exists:
- Windows: `build\chess_engine.exe` or `build\Release\chess_engine.exe`
- Linux/macOS: `./chess_engine` or `./build/chess_engine`

Run it to test:
```bash
# Windows
build\chess_engine.exe

# Linux/macOS
./chess_engine
```

