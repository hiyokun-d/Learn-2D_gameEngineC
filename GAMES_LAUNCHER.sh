
#!/bin/bash

OS=$(uname)
if [[ "$OS" == "Darwin" ]]; then
    PM="brew"
elif [[ "$OS" == "Linux" ]]; then
    PM="sudo apt"
else
    echo "Unsupported OS."
    exit 1
fi

check_dep() {
    command -v $1 >/dev/null || { echo "Installing $1..."; $PM install $1; }
}

check_dep gcc
check_dep sdl2-config

while true; do
    clear
    echo "🎮 Game Selector by Hiyo"
    echo "1. Breakout"
    echo "2. Terminal Maze"
    echo "3. Snake"
    echo "4. Exit"
    read -p "Select a game (1-4): " choice
    case $choice in
        1) ./runner.sh ;;
        2) gcc ./terminal_game.c -o terminal_game.out && ./terminal_game.out ;;
        3) ./snake_runner.sh ;;
        4) echo "Goodbye!"; exit 0 ;;
        *) echo "Invalid choice." ;;
    esac
    read -p "Press ENTER to continue..."
done
