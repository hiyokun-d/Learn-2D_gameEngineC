@echo off
echo Compiling Terminal Maze Game...
gcc ../terminal_game.c -o terminal_maze.exe -O2 -Wall
if %ERRORLEVEL% neq 0 (
    echo ERROR: Compilation failed!
    echo Please install MinGW or another C compiler.
    pause
    exit /b
)
echo Terminal Maze Game compiled successfully!
echo You can now run terminal_maze.exe
pause
