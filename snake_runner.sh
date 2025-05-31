#!/bin/bash
gcc snake_game.c -o snake_game $(sdl2-config --cflags --libs) -lSDL2_ttf && ./snake_game

