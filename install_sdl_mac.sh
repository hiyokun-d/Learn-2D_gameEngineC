#!/bin/bash
# Script to install SDL2, SDL2_ttf, and SDL2_mixer on macOS using Homebrew

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}SDL2 Libraries Installation Script for macOS${NC}"
echo "======================================"
echo 

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo -e "${YELLOW}Homebrew not found. Installing Homebrew...${NC}"
    # Install Homebrew
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    
    # Add Homebrew to PATH if it's not already there
    if [[ $(uname -m) == "arm64" ]]; then
        echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
        eval "$(/opt/homebrew/bin/brew shellenv)"
    else
        echo 'eval "$(/usr/local/bin/brew shellenv)"' >> ~/.zprofile
        eval "$(/usr/local/bin/brew shellenv)"
    fi
    
    echo -e "${GREEN}Homebrew installed successfully!${NC}"
else
    echo -e "${GREEN}Homebrew is already installed. Proceeding...${NC}"
fi

# Update Homebrew
echo "Updating Homebrew..."
brew update

# Install SDL2
echo -e "\n${GREEN}Installing SDL2...${NC}"
brew install sdl2
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to install SDL2. Please check the error message above.${NC}"
    exit 1
fi
echo -e "${GREEN}SDL2 installed successfully!${NC}"

# Install SDL2_ttf
echo -e "\n${GREEN}Installing SDL2_ttf...${NC}"
brew install sdl2_ttf
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to install SDL2_ttf. Please check the error message above.${NC}"
    exit 1
fi
echo -e "${GREEN}SDL2_ttf installed successfully!${NC}"

# Install SDL2_mixer
echo -e "\n${GREEN}Installing SDL2_mixer...${NC}"
brew install sdl2_mixer
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to install SDL2_mixer. Please check the error message above.${NC}"
    exit 1
fi
echo -e "${GREEN}SDL2_mixer installed successfully!${NC}"

# Verify installation
echo -e "\n${GREEN}Verifying installation...${NC}"
SDL2_PATH=$(brew --prefix sdl2)
SDL2_TTF_PATH=$(brew --prefix sdl2_ttf)
SDL2_MIXER_PATH=$(brew --prefix sdl2_mixer)

echo -e "SDL2 installed at: ${YELLOW}$SDL2_PATH${NC}"
echo -e "SDL2_ttf installed at: ${YELLOW}$SDL2_TTF_PATH${NC}"
echo -e "SDL2_mixer installed at: ${YELLOW}$SDL2_MIXER_PATH${NC}"

# Print compilation instructions
echo -e "\n${GREEN}Installation complete!${NC}"
echo -e "To compile a program with SDL2, SDL2_ttf, and SDL2_mixer, use:"
echo -e "${YELLOW}gcc your_program.c -o your_program \$(sdl2-config --cflags --libs) -lSDL2_ttf -lSDL2_mixer${NC}"
echo 
echo -e "For the games in this project, you can use:"
echo -e "${YELLOW}gcc main.c -o breakout \$(sdl2-config --cflags --libs) -lSDL2_ttf -lSDL2_mixer${NC}"
echo -e "${YELLOW}gcc snake_game.c -o snake_game \$(sdl2-config --cflags --libs) -lSDL2_ttf -lSDL2_mixer${NC}"
echo 
echo -e "${GREEN}Happy coding!${NC}"

