#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 400

typedef struct {
  int x, y, w, h;
  SDL_Color color; // property off RGBA which is stands for (RED, GREEN, BLUE
                   // AND ALPHA)
} Rectangle;

void drawRectangle(SDL_Renderer *renderer, Rectangle rectangle) {
  SDL_Rect rect;
  rect.x = rectangle.x;
  rect.y = rectangle.y;
  rect.w = rectangle.w;
  rect.h = rectangle.h;

  SDL_SetRenderDrawColor(renderer, rectangle.color.r, rectangle.color.g,
                         rectangle.color.b,
                         rectangle.color.a); // Light blue color
  SDL_RenderFillRect(renderer, &rect);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0,
                         255); // Set back to black for future draws
}

int player_X = SCREEN_WIDTH / 2 - 20;
int player_Y = SCREEN_HEIGHT / 2 - 20;
int main() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }

  // Create the window
  SDL_Window *window = SDL_CreateWindow(
      "2D Game Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

  if (window == NULL) {
    SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }

  // Create the renderer
  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    SDL_Log("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }

  // Game loop
  bool isRunning = true;
  SDL_Event event;

  while (isRunning) {
    Rectangle playerBlock = {player_X, player_Y, 20, 20, {23, 231, 255, 255}};
    Rectangle testBlock = {20, 50, 50, 50, {200, 150, 20, 255}};

    // Event handling
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        isRunning = false;
      }

      // PENGGANTI ADDEVENTLISTENER DI JAVASCRIPT
      switch (event.type) {
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_a && player_X >= 10) {
          player_X -= 10;
        } else if (event.key.keysym.sym == SDLK_d &&
                   player_X <= SCREEN_WIDTH - playerBlock.w - 10) {
          player_X += 10;
        }

        if (event.key.keysym.sym == SDLK_w && player_Y >= 10) {
          player_Y -= 10;
        } else if (event.key.keysym.sym == SDLK_s &&
                   player_Y <= SCREEN_HEIGHT - playerBlock.h - 10) {
          player_Y += 10;
        }
        break;

      case SDL_KEYUP:
        // WRITE SOME CODE IN THIS
        break;
      }
    }

    // Horizontal Collision
    if (!(playerBlock.x > testBlock.x + testBlock.w ||
          playerBlock.x + playerBlock.w < testBlock.x ||
          playerBlock.y > testBlock.y + testBlock.h ||
          playerBlock.y + playerBlock.h < testBlock.y)) {
    }

    // Clear the screen with black
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    drawRectangle(renderer, playerBlock);
    drawRectangle(renderer, testBlock);

    SDL_RenderPresent(renderer);
  }

  // Cleanup
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  printf("Goodbye World!\n");

  return 0;
}
