=================================================
       2D Game Collection for Windows
=================================================

This package contains two SDL2-based games:

1. Breakout Game - A classic brick-breaking game
2. Snake Game - Control a snake to eat food and grow

QUICK START:
-----------
1. Run 'setup_games.bat' first to copy DLLs to the correct location
2. Run 'run_games.bat' to play the games

IMPORTANT:
---------
If the games are not already compiled (.exe files not in the bin folder),
you will need to compile them on Windows:

1. Install MinGW:
   - Download and install MinGW from: https://sourceforge.net/projects/mingw/
   - During installation, select the "mingw32-base" and "mingw32-gcc-g++" packages
   - Add MinGW's bin directory to your system PATH

2. Install SDL2 Development Libraries:
   - Download SDL2 development libraries for MinGW from:
     https://www.libsdl.org/download-2.0.php
   - Download SDL2_ttf development libraries from:
     https://www.libsdl.org/projects/SDL_ttf/
   - Extract both to a folder (e.g., C:\SDL2)

3. Compile the games:
   - Open a command prompt in this directory
   - Run the following commands (adjust paths as needed):

     gcc main.c -o bin\breakout.exe -I"C:\SDL2\include" -L"C:\SDL2\lib" -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -O2
     gcc snake_game.c -o bin\snake_game.exe -I"C:\SDL2\include" -L"C:\SDL2\lib" -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -O2

CONTROLS:
--------
- Breakout: A/D keys to move the paddle
- Snake: Arrow keys or WASD to change direction

=================================================
