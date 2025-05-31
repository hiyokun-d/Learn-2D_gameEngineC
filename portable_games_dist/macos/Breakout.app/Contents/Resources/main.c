#include <SDL.h>
#include <SDL_ttf.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 400

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

int totalBall = 35;
int score = 0;

bool automatic_paddle = false;

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

typedef struct BlockNode {
  Rectangle block;
  struct BlockNode *next;
} BlockNode;

// Function prototypes
void drawRectangle(SDL_Renderer *renderer, Rectangle rectangle);

double degreesToRadians(double degrees) { return degrees * M_PI / 180.0; }

BlockNode *createBlockList(int size) {
  BlockNode *head = NULL;
  BlockNode *tail = NULL;

  int blockWidth = 50;
  int blockHeight = 20;
  int padding = 10;
  int blocksPerRow = SCREEN_WIDTH / (blockWidth + padding);

  for (int i = 0; i < size; i++) {
    BlockNode *newNode = (BlockNode *)malloc(sizeof(BlockNode));
    newNode->block.x = (i % blocksPerRow) * (blockWidth + padding) + 5;
    newNode->block.y = (i / blocksPerRow) * (blockHeight + padding);
    newNode->block.w = blockWidth;
    newNode->block.h = blockHeight;
    newNode->block.color.r = rand() % 256;
    newNode->block.color.g = rand() % 256;
    newNode->block.color.b = rand() % 256;
    newNode->block.color.a = 255;
    newNode->next = NULL;

    if (head == NULL) {
      head = newNode;
      tail = newNode;
    } else {
      tail->next = newNode;
      tail = newNode;
    }
  }

  return head;
}

void drawBlocks(SDL_Renderer *renderer, BlockNode *head) {
  BlockNode *current = head;
  while (current != NULL) {
    drawRectangle(renderer, current->block);
    current = current->next;
  }
}

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

void breakBlock(struct Arc ball, BlockNode **head) {
  BlockNode *current = *head;
  BlockNode *prev = NULL;

  while (current != NULL) {
    Rectangle blk = current->block;
    // Simple AABB (Axis-Aligned Bounding Box) collision
    if (ball.x + ball.r > blk.x && ball.x - ball.r < blk.x + blk.w &&
        ball.y + ball.r > blk.y && ball.y - ball.r < blk.y + blk.h) {

      // Collision detected – remove the block
      ball_vy = ballSpeed;
      score++;
      totalBall--;

      if (prev == NULL) {
        *head = current->next;
      } else {
        prev->next = current->next;
      }
      BlockNode *temp = current;
      current = current->next;
      free(temp);
      return; // Remove one block per frame
    } else {
      prev = current;
      current = current->next;
    }
  }
}

void renderWinMessage(SDL_Renderer *renderer, TTF_Font *font, char *condition) {
  SDL_Color green = {0, 255, 0, 255};
  SDL_Surface *textSurface = TTF_RenderText_Solid(font, condition, green);
  SDL_Texture *textTexture =
      SDL_CreateTextureFromSurface(renderer, textSurface);

  SDL_Rect textRect = {SCREEN_WIDTH / 2 - textSurface->w / 2,
                       SCREEN_HEIGHT / 2 - textSurface->h / 2, textSurface->w,
                       textSurface->h};
  SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

  SDL_FreeSurface(textSurface);
  SDL_DestroyTexture(textTexture);
}

void renderScore(SDL_Renderer *renderer, TTF_Font *font, int score) {
  char scoreText[32];
  sprintf(scoreText, "Score: %d", score);

  SDL_Color white = {255, 255, 255, 255};
  SDL_Surface *textSurface = TTF_RenderText_Solid(font, scoreText, white);
  SDL_Texture *textTexture =
      SDL_CreateTextureFromSurface(renderer, textSurface);

  SDL_Rect textRect = {10, SCREEN_HEIGHT - 25, textSurface->w, textSurface->h};
  SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

  SDL_FreeSurface(textSurface);
  SDL_DestroyTexture(textTexture);
}

int main() {

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }

  if (TTF_Init() == -1) {
    printf("TTF_Init: %s\n", TTF_GetError());
    return -1;
  }

  // Load a font (you must have this TTF file)
  TTF_Font *font = TTF_OpenFont("arial.ttf", 24);
  if (!font) {
    printf("Failed to load font: %s\n", TTF_GetError());
    SDL_Quit();
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

  BlockNode *blockList = createBlockList(totalBall);

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

    // BOTTOM OF THE SCREEN OR SOMETHING, REMINDER: don't get fucked up AND
    // CHANGE ANYTHING IN THIS CODE YOU STUPID ASS

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

    bool playerWon = (totalBall == 0);

    renderScore(renderer, font, score);

    if (ball_y < 0 + ball.r) {
      ball_vy = ballSpeed;
    }

    if (!playerWon && ball_y > SCREEN_HEIGHT) {
      renderWinMessage(renderer, font, "lose");
    }

    if (playerWon) {
      renderWinMessage(renderer, font, "win");
    }

    breakBlock(ball, &blockList);

    drawArc(renderer, ball);
    // for (int i = 0; i < size; i++) {
    //   drawRectangle(renderer, block[i]);
    // }
    //
    drawBlocks(renderer, blockList);

    renderScore(renderer, font, score);
    SDL_RenderPresent(renderer);
    SDL_Delay(1000 / 60);
  }

  // Cleanup
  // Free the block list
  BlockNode *current = blockList;
  while (current != NULL) {
    BlockNode *next = current->next;
    free(current);
    current = next;
  }

  TTF_CloseFont(font);
  TTF_Quit();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  printf("Goodbye World!\n");

  return 0;
}
