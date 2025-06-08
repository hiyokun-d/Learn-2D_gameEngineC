#!/bin/bash
# Game Selector for Linux/macOS

# Check for available games
HAVE_BREAKOUT=0
HAVE_TERMINAL=0
HAVE_SNAKE=0

[[ -f breakout ]] && HAVE_BREAKOUT=1
[[ -f terminal_maze ]] && HAVE_TERMINAL=1
[[ -f snake ]] && HAVE_SNAKE=1

while true; do
  clear
  echo "==========================================="
  echo "         🎮 GAME SELECTOR by Hiyo"
  echo "==========================================="
  echo
  echo "Please select a game to play:"
  echo
  echo "1. Breakout Game (brick-breaking game with power-ups)"
  [[ $HAVE_BREAKOUT -eq 0 ]] && echo "   [NOT FOUND]"

  echo "2. Terminal Maze (text-based maze navigation)"
  [[ $HAVE_TERMINAL -eq 0 ]] && echo "   [NOT FOUND]"

  echo "3. Snake Game (classic snake game)"
  [[ $HAVE_SNAKE -eq 0 ]] && echo "   [NOT FOUND]"

  echo "4. Exit"
  echo

  read -p "Enter your choice (1-4): " choice

  case $choice in
    1)
      if [[ $HAVE_BREAKOUT -eq 1 ]]; then
        echo "Starting Breakout Game..."
        ./breakout
      else
        echo "Breakout not found. Would you like to compile it? (Y/N)"
        read -r confirm
        if [[ $confirm == [Yy] ]]; then
          gcc breakout_game.c -o breakout $(sdl2-config --cflags --libs) -lSDL2_ttf
          echo "Compiled breakout. Run again to play."
        fi
      fi
      ;;
    2)
      if [[ $HAVE_TERMINAL -eq 1 ]]; then
        echo "Starting Terminal Maze..."
        ./terminal_maze
      else
        echo "Terminal Maze not found. Would you like to compile it? (Y/N)"
        read -r confirm
        if [[ $confirm == [Yy] ]]; then
          gcc terminal_game.c -o terminal_maze
          echo "Compiled terminal_maze. Run again to play."
        fi
      fi
      ;;
    3)
      if [[ $HAVE_SNAKE -eq 1 ]]; then
        echo "Starting Snake Game..."
        ./snake
      else
        echo "Snake not found. Would you like to compile it? (Y/N)"
        read -r confirm
        if [[ $confirm == [Yy] ]]; then
          gcc snake_game.c -o snake $(sdl2-config --cflags --libs) -lSDL2_ttf
          echo "Compiled snake. Run again to play."
        fi
      fi
      ;;
    4)
      echo "Exiting the game selector. This game was made by Hiyo with love."
      exit 0
      ;;
    *)
      echo "Invalid choice. Please enter a number from 1 to 4."
      ;;
  esac

  echo
  read -p "Press ENTER to return to the menu..."
done