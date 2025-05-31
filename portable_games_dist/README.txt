=================================================
   2D Game Collection for Windows and macOS
=================================================

This package contains portable games that run on both 
Windows and macOS:

TERMINAL GAME (No dependencies):
------------------------------
The 'terminal_game' folder contains a maze navigation game
that runs in the terminal/command prompt without any external
dependencies. Works on both Windows and macOS.

WINDOWS GAMES (SDL2-based):
-------------------------
The 'windows' folder contains:
- Breakout Game - A classic brick-breaking game
- Snake Game - Control a snake to eat food and grow

These games require the SDL2 DLLs included in the package.
See the README.txt in the windows folder for instructions.

MACOS GAMES (SDL2-based):
-----------------------
The 'macos' folder contains:
- Breakout.app - A classic brick-breaking game
- SnakeGame.app - Control a snake to eat food and grow

These games have SDL2 frameworks embedded in the .app bundles,
so they should run without requiring SDL2 to be installed.
See the README.txt in the macos folder for instructions.

=================================================
