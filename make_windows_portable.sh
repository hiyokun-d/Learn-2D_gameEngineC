#!/bin/bash
# Script to create a portable Windows distribution of the games
# No setup will be required on the Windows machine - just unzip and run

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${GREEN}Creating portable Windows distribution...${NC}"

# Create output directory
DIST_DIR="BreakoutGames_Windows"
mkdir -p "$DIST_DIR"

# Check for MinGW cross-compiler
if command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo -e "${GREEN}MinGW cross-compiler found. Will compile for Windows.${NC}"
    CAN_CROSS_COMPILE=true
else
    echo -e "${YELLOW}MinGW cross-compiler not found. Will create a distribution without executables.${NC}"
    echo -e "${YELLOW}User will need to compile the games on Windows.${NC}"
    CAN_CROSS_COMPILE=false
fi

# Download SDL2 DLLs for Windows
echo -e "${BLUE}Downloading SDL2 DLLs...${NC}"

# Create DLLs directory
mkdir -p "$DIST_DIR/DLLs"

# SDL2 DLL URLs
SDL2_URL="https://www.libsdl.org/release/SDL2-2.0.22-win32-x86.zip"
SDL2_TTF_URL="https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.18-win32-x86.zip"
SDL2_MIXER_URL="https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.4-win32-x86.zip"

# Download and extract SDL2
echo -e "Downloading SDL2 DLLs..."
curl -L "$SDL2_URL" -o "$DIST_DIR/sdl2.zip"
unzip -j "$DIST_DIR/sdl2.zip" "*.dll" -d "$DIST_DIR/DLLs"
rm "$DIST_DIR/sdl2.zip"

# Download and extract SDL2_ttf
echo -e "Downloading SDL2_ttf DLLs..."
curl -L "$SDL2_TTF_URL" -o "$DIST_DIR/sdl2_ttf.zip"
unzip -j "$DIST_DIR/sdl2_ttf.zip" "*.dll" -d "$DIST_DIR/DLLs"
rm "$DIST_DIR/sdl2_ttf.zip"

# Download and extract SDL2_mixer
echo -e "Downloading SDL2_mixer DLLs..."
curl -L "$SDL2_MIXER_URL" -o "$DIST_DIR/sdl2_mixer.zip"
unzip -j "$DIST_DIR/sdl2_mixer.zip" "*.dll" -d "$DIST_DIR/DLLs"
rm "$DIST_DIR/sdl2_mixer.zip"

# Copy sound files
echo -e "${BLUE}Copying sound files...${NC}"
mkdir -p "$DIST_DIR/sounds"
cp -r sounds/* "$DIST_DIR/sounds/"

# Copy font file
echo -e "${BLUE}Copying font file...${NC}"
if [ -f "Arial.ttf" ]; then
    cp "Arial.ttf" "$DIST_DIR/"
else
    echo -e "${YELLOW}Arial.ttf not found. Downloading a free alternative...${NC}"
    curl -L "https://github.com/googlefonts/roboto/raw/main/src/hinted/Roboto-Regular.ttf" -o "$DIST_DIR/Arial.ttf"
fi

# Copy source files
echo -e "${BLUE}Copying source files...${NC}"
mkdir -p "$DIST_DIR/src"

# Check if main.c exists, if not create a dummy file
if [ -f "main.c" ]; then
    cp "main.c" "$DIST_DIR/src/"
else
    echo -e "${YELLOW}main.c not found. Creating a placeholder...${NC}"
    echo '// Placeholder for main.c - this file was missing during packaging' > "$DIST_DIR/src/main.c"
fi

# Check if terminal_game.c exists, if not create a dummy file
if [ -f "terminal_game.c" ]; then
    cp "terminal_game.c" "$DIST_DIR/src/"
else
    echo -e "${YELLOW}terminal_game.c not found. Creating a placeholder...${NC}"
    echo '// Placeholder for terminal_game.c - this file was missing during packaging' > "$DIST_DIR/src/terminal_game.c"
fi

# Cross-compile if possible
if [ "$CAN_CROSS_COMPILE" = true ]; then
    echo -e "${BLUE}Cross-compiling games for Windows...${NC}"
    
    # Set up MinGW compiler paths
    MINGW_PATH=$(which x86_64-w64-mingw32-gcc)
    MINGW_DIR=$(dirname "$MINGW_PATH")
    
    # SDL includes and libs paths (use defaults if not found)
    SDL2_PATH=$(brew --prefix sdl2 2>/dev/null || echo "/usr/local/opt/sdl2")
    SDL2_TTF_PATH=$(brew --prefix sdl2_ttf 2>/dev/null || echo "/usr/local/opt/sdl2_ttf")
    SDL2_MIXER_PATH=$(brew --prefix sdl2_mixer 2>/dev/null || echo "/usr/local/opt/sdl2_mixer")
    
    # Compilation flags
    SDL_CFLAGS="-I$SDL2_PATH/include/SDL2 -I$SDL2_TTF_PATH/include/SDL2 -I$SDL2_MIXER_PATH/include/SDL2 -Dmain=SDL_main"
    SDL_LDFLAGS="-L$SDL2_PATH/lib -L$SDL2_TTF_PATH/lib -L$SDL2_MIXER_PATH/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_mixer"
    
    # Compile main.c (Breakout game)
    if [ -f "main.c" ]; then
        echo -e "Compiling Breakout game..."
        x86_64-w64-mingw32-gcc "main.c" -o "$DIST_DIR/breakout.exe" $SDL_CFLAGS $SDL_LDFLAGS -O2
        if [ $? -ne 0 ]; then
            echo -e "${YELLOW}Compilation failed. Will provide instructions instead.${NC}"
        else
            echo -e "${GREEN}Breakout game compiled successfully!${NC}"
        fi
    fi
    
    # Compile terminal_game.c (Terminal Maze game)
    if [ -f "terminal_game.c" ]; then
        echo -e "Compiling Terminal Maze game..."
        x86_64-w64-mingw32-gcc "terminal_game.c" -o "$DIST_DIR/terminal_maze.exe" -O2
        if [ $? -ne 0 ]; then
            echo -e "${YELLOW}Compilation failed. Will provide instructions instead.${NC}"
        else
            echo -e "${GREEN}Terminal Maze game compiled successfully!${NC}"
        fi
    fi
else
    echo -e "${YELLOW}Skipping cross-compilation due to missing MinGW.${NC}"
fi

# Create launcher batch file
echo -e "${BLUE}Creating launcher batch files...${NC}"

# Main launcher
cat > "$DIST_DIR/Play_Games.bat" << 'EOF'
@echo off
echo ======================================
echo      Breakout Games Collection
echo ======================================
echo.
echo  1. Play Breakout
echo  2. Play Terminal Maze
echo  3. Exit
echo.
echo ======================================

choice /C 123 /N /M "Choose a game to play (1-3): "

if %ERRORLEVEL% == 1 goto breakout
if %ERRORLEVEL% == 2 goto maze
if %ERRORLEVEL% == 3 goto end

:breakout
echo Starting Breakout Game...
if exist breakout.exe (
    start breakout.exe
) else (
    echo Executable not found! Please compile the game first.
    echo See the README.txt for instructions.
    pause
)
goto end

:maze
echo Starting Terminal Maze Game...
if exist terminal_maze.exe (
    start terminal_maze.exe
) else (
    echo Executable not found! Please compile the game first.
    echo See the README.txt for instructions.
    pause
)
goto end

:end
EOF

# Create setup script to move DLLs
cat > "$DIST_DIR/Setup.bat" << 'EOF'
@echo off
echo Setting up games...
echo.

echo Copying DLLs to main directory...
copy /Y DLLs\*.dll .
echo.

echo Setup complete!
echo You can now run the games using Play_Games.bat
pause
EOF

# Create compilation instructions for Windows
cat > "$DIST_DIR/Compile_Games.bat" << 'EOF'
@echo off
echo Compiling games for Windows...
echo.

REM Check if we have gcc installed
where gcc >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: GCC compiler not found!
    echo Please install MinGW or another C compiler.
    echo See the README.txt file for more information.
    pause
    exit /b
)

echo Compiling Breakout Game...
gcc src\main.c -o breakout.exe -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_mixer
if %ERRORLEVEL% neq 0 (
    echo ERROR: Compilation failed!
    echo Make sure SDL2 development libraries are installed.
    pause
    exit /b
)
echo Breakout Game compiled successfully!

echo.
echo Compiling Terminal Maze Game...
gcc src\terminal_game.c -o terminal_maze.exe
if %ERRORLEVEL% neq 0 (
    echo ERROR: Compilation failed!
    pause
    exit /b
)
echo Terminal Maze Game compiled successfully!

echo.
echo All games compiled! You can now run Play_Games.bat
pause
EOF

# Create README.txt with instructions
cat > "$DIST_DIR/README.txt" << 'EOF'
============================================
        BREAKOUT GAMES COLLECTION
============================================

This package contains:

1. Breakout Game - A classic brick-breaking game with power-ups
2. Terminal Maze - A text-based maze navigation game

QUICK START:
-----------
1. Run "Setup.bat" first to copy DLLs to the main directory
2. Run "Play_Games.bat" to start playing

If the executables are missing or don't work:
- Run "Compile_Games.bat" if you have a C compiler installed on Windows
- OR download pre-compiled executables from the project website

CONTROLS:
--------
Breakout:
- A/D: Move paddle left/right
- Space: Launch ball
- P: Pause game
- M: Toggle mouse control
- F: Toggle auto-paddle
- ESC: Return to menu

Terminal Maze:
- WASD: Move
- Q: Quit
- R: Restart

TROUBLESHOOTING:
--------------
If the games don't run:
1. Make sure you ran Setup.bat first
2. Check that all DLL files are in the same folder as the EXE files
3. Make sure your graphics drivers are up to date

CREDITS:
-------
Game developed for educational purposes
Sound effects from OpenGameArt.org (CC0 License)
============================================
EOF

# Package everything into a ZIP file
echo -e "${BLUE}Creating ZIP archive...${NC}"
zip -r "${DIST_DIR}.zip" "$DIST_DIR"

echo -e "${GREEN}Portable Windows distribution created successfully!${NC}"
echo -e "Distribution directory: ${GREEN}$DIST_DIR${NC}"
echo -e "Distribution archive: ${GREEN}${DIST_DIR}.zip${NC}"
echo -e "\nTo use on Windows:"
echo -e "1. Copy the ZIP file to a Windows computer"
echo -e "2. Extract the ZIP file"
echo -e "3. Run Setup.bat first"
echo -e "4. Run Play_Games.bat to start playing"

