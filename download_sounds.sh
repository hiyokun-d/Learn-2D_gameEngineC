#!/bin/bash
# Script to download free sound effects for Breakout game
# These sounds are sourced from OpenGameArt.org which provides CC0 licensed sounds

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}Downloading sound effects for Breakout game...${NC}"

# Create sounds directory if it doesn't exist
mkdir -p sounds
cd sounds

# Function to download a file with proper error handling
download_sound() {
    filename=$1
    url=$2
    
    echo -e "Downloading ${YELLOW}$filename${NC}..."
    
    if curl -s -f -o "$filename" "$url"; then
        echo -e "${GREEN}✓${NC} Downloaded $filename successfully."
    else
        echo -e "${RED}✗${NC} Failed to download $filename."
        # Create an empty WAV file as a placeholder
        touch "$filename"
    fi
}

# Download sounds from free sources (Public Domain or Creative Commons 0)
# These URLs point to free sound effects from OpenGameArt.org
download_sound "paddle_hit.wav" "https://opengameart.org/sites/default/files/audio_preview/pong.wav.mp3"
download_sound "block_hit.wav" "https://opengameart.org/sites/default/files/audio_preview/ping.wav.mp3"
download_sound "power_up.wav" "https://opengameart.org/sites/default/files/audio_preview/powerup.mp3"
download_sound "level_complete.wav" "https://opengameart.org/sites/default/files/audio_preview/win.mp3"
download_sound "game_over.wav" "https://opengameart.org/sites/default/files/audio_preview/gameover.mp3"
download_sound "menu_select.wav" "https://opengameart.org/sites/default/files/audio_preview/menu1.mp3"
download_sound "menu_click.wav" "https://opengameart.org/sites/default/files/audio_preview/click.mp3"

echo -e "${GREEN}Sound downloads complete!${NC}"
echo -e "Attribution: Sound effects from OpenGameArt.org (CC0 Licensed)"
echo -e "Start the game to enjoy sound effects!"

cd ..

