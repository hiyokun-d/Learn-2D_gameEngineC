@echo off
echo ======================================
echo      2D Game Collection for Windows
echo ======================================
echo.
echo Before running games:
echo - Copy all DLLs from 'dlls' folder to 'bin' folder
echo.
echo  1. Play Breakout Game
echo  2. Play Snake Game
echo  3. Exit
echo.
echo ======================================

choice /C 123 /N /M "Choose a game to play (1-3): "

if %ERRORLEVEL% == 1 goto breakout
if %ERRORLEVEL% == 2 goto snake
if %ERRORLEVEL% == 3 goto end

:breakout
echo Starting Breakout Game...
if exist bin\breakout.exe (
    cd bin
    breakout.exe
    cd ..
) else (
    echo ERROR: breakout.exe not found! 
    echo Please compile the game first.
    pause
)
goto end

:snake
echo Starting Snake Game...
if exist bin\snake_game.exe (
    cd bin
    snake_game.exe
    cd ..
) else (
    echo ERROR: snake_game.exe not found!
    echo Please compile the game first.
    pause
)
goto end

:end
