#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 400

typedef struct {
  int x, y, w, h;
  SDL_Color color; // property off RGBA which is stands for (RED, GREEN, BLUE
                   // AND ALPHA)
} Rectangle;

struct Arc {
  int x, y;
  int r;
  double startAngle;
  double endAngle;
  SDL_Color color;
};

double degreesToRadians(double degrees) { return degrees * M_PI / 180.0; }

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

void drawArc(SDL_Renderer *renderer, struct Arc arc) {
  SDL_SetRenderDrawColor(renderer, arc.color.r, arc.color.g, arc.color.b,
                         arc.color.a);

  for (int w = 0; w < arc.r * 2; w++) {
    for (int h = 0; h < arc.r * 2; h++) {
      int dx = arc.r - w;
      int dy = arc.r - h;
      double distanceSquared = dx * dx + dy * dy;

      if (distanceSquared <= arc.r * arc.r) {
        double angle = atan2(dy, dx);

        if (angle < 0) {
          angle += 2 * M_PI;
        }

        if (angle >= arc.startAngle && angle <= arc.endAngle) {
          SDL_RenderDrawPoint(renderer, arc.x + dx, arc.y + dy);
        }
      }
    }
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

int checkCollision(struct Arc ball, Rectangle player) {
  // Check if the ball's position is within the player's bounds horizontally
  if (ball.x + ball.r > player.x && ball.x - ball.r < player.x + player.w) {
    // Check if the ball's position is within the player's bounds vertically
    if (ball.y + ball.r > player.y && ball.y - ball.r < player.y + player.h) {
      return 1; // Collision detected
    }
  }
  return 0; // No collision
}

void makingBlock(Rectangle block[], int size) {
  int blockWidth = 50;
  int blockHeight = 20;
  int padding = 10;

  for (int i = 0; i < size; i++) {
    int blocksPerRow = SCREEN_WIDTH / (blockWidth + padding);

    block[i].x = (i % blocksPerRow) * (blockWidth + padding) + 5;
    block[i].y = (i / blocksPerRow) * (blockHeight + padding);
    block[i].w = blockWidth;
    block[i].h = blockHeight;
    block[i].color.r = rand() % 256;
    block[i].color.g = rand() % 256;
    block[i].color.b = rand() % 256;
    block[i].color.a = 255;
  }
}

float player_X = SCREEN_WIDTH / 2 - 20;
float player_Y = SCREEN_HEIGHT - (20 * 3);
const float playerSpeed = 9.7;
float player_vy = 0;
float player_vx = 0;

float ball_x = SCREEN_WIDTH / 2 - 10;
float ball_y = SCREEN_HEIGHT / 2 - 10;
const float ballSpeed = 5.0;
float ball_vx = ballSpeed;
float ball_vy = ballSpeed;

bool automatic_paddle = false;

int main() {

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }

  SDL_Window *window = SDL_CreateWindow(
      "2D Game Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

  if (window == NULL) {
    SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }

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

  Rectangle block[40];
  int size = sizeof(block) / sizeof(block[0]);
  makingBlock(block, size);
  while (isRunning) {
    Rectangle playerBlock = {player_X, player_Y, 90, 20, {23, 231, 255, 255}};
    struct Arc ball = {ball_x, ball_y, 10, 0, M_PI * 2, {255, 0, 0, 255}};

    // Event handling
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        isRunning = false;
      }

      switch (event.type) {
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_a) {
          player_vx = -playerSpeed;
        } else if (event.key.keysym.sym == SDLK_d) {
          player_vx = playerSpeed;
        }

        if (event.key.keysym.sym == SDLK_f && !event.key.repeat) {
          if (!automatic_paddle) {
            automatic_paddle = true;
          } else
            automatic_paddle = false;
        }

        break;

      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_d) {
          player_vx = 0;
        }

        // if (event.key.keysym.sym == SDLK_w || event.key.keysym.sym ==
        // SDLK_s)
        // {
        //   player_vy = 0;
        // }

        break;
      }
    }

    ball_x += ball_vx;
    ball_y += ball_vy;
    if (checkCollision(ball, playerBlock)) {
      ball_vy = -ballSpeed;
    }

    if (ball_x < 0) {
      ball_vx = ballSpeed;
    }

    if (ball_x > SCREEN_WIDTH - ball.r) {
      ball_vx = -ballSpeed;
    }

    if (ball_y < 0) {
      ball_vy = ballSpeed;
    }

    if (ball_y > SCREEN_HEIGHT - ball.r) {
      ball_vy = -ballSpeed;
    }

    player_X += player_vx;
    player_Y += player_vy;

    if (automatic_paddle) {
      player_X += ball_vx;
    }

    if (player_X < 0) {
      player_X = 0;
    }
    if (player_X > SCREEN_WIDTH - playerBlock.w) {
      player_X = SCREEN_WIDTH - playerBlock.w; // Right boundary
    }
    if (player_Y < 0) {
      player_Y = 0; // Top boundary
    }
    if (player_Y > SCREEN_HEIGHT - playerBlock.h) {
      player_Y = SCREEN_HEIGHT - playerBlock.h; // Bottom boundary
    }

    // Horizontal Collision
    // if (!(playerBlock.x > testBlock.x + testBlock.w ||
    //       playerBlock.x + playerBlock.w < testBlock.x ||
    //       playerBlock.y > testBlock.y + testBlock.h ||
    //       playerBlock.y + playerBlock.h < testBlock.y)) {
    // }

    // Clear the screen with black
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    drawRectangle(renderer, playerBlock);
    drawArc(renderer, ball);
    for (int i = 0; i < size; i++) {
      drawRectangle(renderer, block[i]);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(1000 / 60);
  }

  // Cleanup
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  printf("Goodbye World!\n");

  return 0;
}
