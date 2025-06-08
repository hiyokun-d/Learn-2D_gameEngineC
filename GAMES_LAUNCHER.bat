@echo off
setlocal EnableDelayedExpansion

:: Check for available games
set HAVE_BREAKOUT=0
set HAVE_TERMINAL=0
set HAVE_SNAKE=0

if exist "breakout.exe" set HAVE_BREAKOUT=1
if exist "terminal_maze.exe" set HAVE_TERMINAL=1
if exist "snake.exe" set HAVE_SNAKE=1

:menu
cls
echo ===========================================
echo         🎮 GAME SELECTOR by Hiyo
echo ===========================================
echo.

echo Please select a game to play:
echo.

echo 1. Breakout Game (brick-breaking game with power-ups)
if "%HAVE_BREAKOUT%"=="0" echo    [NOT FOUND]

echo 2. Terminal Maze (text-based maze navigation)
if "%HAVE_TERMINAL%"=="0" echo    [NOT FOUND]

echo 3. Snake Game (classic snake game)
if "%HAVE_SNAKE%"=="0" echo    [NOT FOUND]

echo 4. Exit
echo.

set /p choice="Enter your choice (1-4): "

if "%choice%"=="1" (
    if "%HAVE_BREAKOUT%"=="1" (
        echo Starting Breakout Game...
        start "" "breakout.exe"
    ) else (
        echo Breakout Game executable not found.
    )
    pause
    goto menu
)

if "%choice%"=="2" (
    if "%HAVE_TERMINAL%"=="1" (
        echo Starting Terminal Maze...
        start "" "terminal_maze.exe"
    ) else (
        echo Terminal Maze executable not found.
    )
    pause
    goto menu
)

if "%choice%"=="3" (
    if "%HAVE_SNAKE%"=="1" (
        echo Starting Snake Game...
        start "" "snake.exe"
    ) else (
        echo Snake Game executable not found.
    )
    pause
    goto menu
)

if "%choice%"=="4" (
    echo Exiting the game selector.
    echo This game was made by Hiyo with love.
    exit /b
)

echo Invalid choice. Please try again.
pause
goto menu