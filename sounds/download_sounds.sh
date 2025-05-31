#!/bin/bash
mkdir -p sounds && cd sounds

# Create simple empty WAV files as placeholders
for sound in paddle_hit block_hit power_up level_complete game_over menu_select menu_click; do
    touch "${sound}.wav"
done

echo "Created placeholder sound files in sounds directory.
Sound will be disabled but the game will run normally."
