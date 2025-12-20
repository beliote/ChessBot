@echo off
REM Simple build script for Windows using g++
REM Requires MinGW or similar g++ installation

echo Building CppChess Engine...

REM Create build directory if it doesn't exist
if not exist "build" mkdir build

REM Compile all source files
g++ -std=c++17 -O3 -Wall -Iinclude -o build\chess_engine.exe src\*.cpp

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful! Executable: build\chess_engine.exe
) else (
    echo.
    echo Build failed! Check errors above.
    exit /b 1
)

