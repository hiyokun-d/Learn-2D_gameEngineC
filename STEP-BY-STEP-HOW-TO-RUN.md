# HOW TO RUN THE GAME

# BREAKOUT GAME

## step 1: just make sure you run this on laptop and not on your phone

## step 2: if you don't have `SDL2` , `SDL2_ttf` ,`SDL2_mixer` installed, install it first in your system by this link [SDL2](https://www.libsdl.org/download-2.0.php)

## step 3: try to run the command of the game in your terminal or you can just run runner.sh

```bash
gcc main.c -o main $(sdl2-config --cflags --libs) -lSDL2_ttf -lSDL2_mixer && ./main
```

## step 4: and if you still have some error issues, you can contact me on discord (hiyo-d) or you can just email me or maybe you can just open an issue on the repository

# SNAKE GAME

## step 1: every step is same but in this case you need to run snake_runner.sh

# for the list of the game

you can just run the `./GAMES_SELECTOR.bat` if you can't run it, just run it manual and if it still can't run it try to fix it with GPT
NOTE: if you still can't run this game pls just ask GPT cause i don't have windows device and i can't test or setup or fix it, but what you need is simple
SDL2, SDL2_ttf, SDL2_mixer if you have those libraries installed and you have the source code of the game, you can run it with this with the file `GAMES_SELECTOR.bat` or you can just run the game manually

```bash
gcc main.c -o main $(sdl2-config --cflags --libs) -lSDL2_ttf -lSDL2_mixer && ./main
```
