@echo off
setlocal EnableDelayedExpansion

ver | findstr /i "Windows" >nul
if errorlevel 1 (
    echo This launcher is for Windows only.
    pause
    exit /b
)

:: SDL2 check
if not exist SDL2.dll (
    echo [!] SDL2.dll not found. Downloading SDL2 runtime...
    powershell -Command ^
        "Invoke-WebRequest -Uri https://github.com/libsdl-org/SDL/releases/download/release-2.30.2/SDL2-2.30.2-win32-x64.zip -OutFile SDL2.zip"
    if not exist SDL2.zip (
        echo Download failed.
        pause
        exit /b
    )
    powershell -Command "Expand-Archive -Force SDL2.zip SDL2_EXTRACTED"
    copy SDL2_EXTRACTED\SDL2-2.30.2\lib\x64\SDL2.dll .
    copy SDL2_EXTRACTED\SDL2-2.30.2\lib\x64\SDL2_ttf.dll .
    del SDL2.zip
    rd /s /q SDL2_EXTRACTED
)

:: Compile if needed
if not exist breakout.exe (
    echo Compiling Breakout...
    gcc main.c -o breakout.exe -IC:\SDL2\include -LC:\SDL2\lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
)

if not exist terminal_maze.exe (
    echo Compiling Terminal Maze...
    gcc terminal_game.c -o terminal_maze.exe -IC:\SDL2\include -LC:\SDL2\lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
)

if not exist snake.exe (
    echo Compiling Snake...
    gcc snake_runner.c -o snake.exe -IC:\SDL2\include -LC:\SDL2\lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
)

:: Update existence flags
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
echo 1. Breakout Game
if "%HAVE_BREAKOUT%"=="0" echo    [NOT FOUND]
echo 2. Terminal Maze
if "%HAVE_TERMINAL%"=="0" echo    [NOT FOUND]
echo 3. Snake Game
if "%HAVE_SNAKE%"=="0" echo    [NOT FOUND]
echo 4. Exit
echo.
set /p choice="Enter your choice (1-4): "
if "%choice%"=="1" (
    if "%HAVE_BREAKOUT%"=="1" (start "" "breakout.exe") else (echo Not found.)
    pause & goto menu
)
if "%choice%"=="2" (
    if "%HAVE_TERMINAL%"=="1" (start "" "terminal_maze.exe") else (echo Not found.)
    pause & goto menu
)
if "%choice%"=="3" (
    if "%HAVE_SNAKE%"=="1" (start "" "snake.exe") else (echo Not found.)
    pause & goto menu
)
if "%choice%"=="4" (
    echo Goodbye!
    exit /b
)
echo Invalid choice. Try again.
pause
goto menu