gcc main.c -o main $(sdl2-config --cflags --libs) -lSDL2_ttf -lSDL2_mixer && ./main
