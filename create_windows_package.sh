#!/bin/bash
mkdir -p Breakout_Windows
# Copy source files
[ -f main.c ] && cp main.c Breakout_Windows/
[ -f terminal_game.c ] && cp terminal_game.c Breakout_Windows/
[ -f Arial.ttf ] && cp Arial.ttf Breakout_Windows/
cp -r sounds Breakout_Windows/

# Create the launcher batch file
cat > Breakout_Windows/PLAY_GAMES.bat << "EOF"
@echo off
echo Downloading required files... Please wait...
powershell -Command "& {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    if (!(Test-Path SDL2.dll)) {
        Invoke-WebRequest -Uri \"https://www.libsdl.org/release/SDL2-2.0.22-win32-x86.zip\" -OutFile sdl2.zip
        Expand-Archive sdl2.zip -DestinationPath . -Force
        Move-Item -Path .\SDL2-2.0.22-win32-x86\SDL2.dll -Destination . -Force
        Remove-Item -Path sdl2.zip -Force
        Remove-Item -Path SDL2-2.0.22-win32-x86 -Recurse -Force
    }
    if (!(Test-Path SDL2_ttf.dll)) {
        Invoke-WebRequest -Uri \"https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.18-win32-x86.zip\" -OutFile sdl2_ttf.zip
        Expand-Archive sdl2_ttf.zip -DestinationPath . -Force
        Move-Item -Path .\SDL2_ttf-2.0.18-win32-x86\SDL2_ttf.dll -Destination . -Force
        Remove-Item -Path sdl2_ttf.zip -Force
        Remove-Item -Path SDL2_ttf-2.0.18-win32-x86 -Recurse -Force
    }
    if (!(Test-Path SDL2_mixer.dll)) {
        Invoke-WebRequest -Uri \"https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.4-win32-x86.zip\" -OutFile sdl2_mixer.zip
        Expand-Archive sdl2_mixer.zip -DestinationPath . -Force
        Move-Item -Path .\SDL2_mixer-2.0.4-win32-x86\SDL2_mixer.dll -Destination . -Force
        Remove-Item -Path sdl2_mixer.zip -Force
        Remove-Item -Path SDL2_mixer-2.0.4-win32-x86 -Recurse -Force
    }
}"

:menu
cls
echo ===============================
echo    BREAKOUT GAMES COLLECTION
echo ===============================
echo.
echo 1. Play Breakout
echo 2. Play Terminal Maze
echo 3. Exit
echo.
choice /C 123 /N /M "Select an option (1-3): "

if errorlevel 3 goto end
if errorlevel 2 goto maze
if errorlevel 1 goto breakout

:breakout
start breakout.exe
goto menu

:maze
start terminal_maze.exe
goto menu

:end
exit
EOF

# Create a README
cat > Breakout_Windows/README.txt << "EOF"
BREAKOUT GAMES COLLECTION
========================

Just double-click PLAY_GAMES.bat to start playing!
The launcher will automatically download any required files.

Controls:
---------
Breakout:
- A/D: Move paddle
- Space: Launch ball
- P: Pause
- ESC: Menu

Terminal Maze:
- WASD: Move
- R: Restart
- Q: Quit
EOF

# Create ZIP file
zip -r Breakout_Windows.zip Breakout_Windows/

echo "Created Breakout_Windows.zip - Windows users just need to:"
echo "1. Extract the ZIP file"
echo "2. Double-click PLAY_GAMES.bat"
echo "Everything else will be set up automatically!"
