#include <SDL.h>
#include <SDL_ttf.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 400

// Game states
typedef enum {
    STATE_LOBBY,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER,
    STATE_WIN
} GameState;

// Menu options
typedef enum {
    MENU_START,
    MENU_EXIT,
    MENU_TOTAL
} MenuOption;

// Game state variables
GameState currentState = STATE_LOBBY;
MenuOption selectedOption = MENU_START;
int highScore = 0;

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

bool game_start = false;
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

// Generic function to render text
void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, 
                SDL_Color color, int x, int y) {
  SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, color);
  if (!textSurface) {
    printf("Failed to render text: %s\n", TTF_GetError());
    return;
  }
  
  SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
  if (!textTexture) {
    printf("Failed to create texture: %s\n", SDL_GetError());
    SDL_FreeSurface(textSurface);
    return;
  }

  SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
  SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

  SDL_FreeSurface(textSurface);
  SDL_DestroyTexture(textTexture);
}

void renderWinMessage(SDL_Renderer *renderer, TTF_Font *font, char *condition) {
  SDL_Color green = {0, 255, 0, 255};
  renderText(renderer, font, condition, green, 
             SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 15);
}

void renderScore(SDL_Renderer *renderer, TTF_Font *font, int score) {
  char scoreText[32];
  sprintf(scoreText, "Score: %d", score);
  SDL_Color white = {255, 255, 255, 255};
  renderText(renderer, font, scoreText, white, 10, SCREEN_HEIGHT - 25);
}

// Render high score
void renderHighScore(SDL_Renderer *renderer, TTF_Font *font, int highScore) {
  if (highScore <= 0) return;
  
  char highScoreText[32];
  sprintf(highScoreText, "High Score: %d", highScore);
  SDL_Color gold = {255, 215, 0, 255}; // Gold color
  renderText(renderer, font, highScoreText, gold, SCREEN_WIDTH - 150, SCREEN_HEIGHT - 25);
}

// Render the lobby screen
void renderLobby(SDL_Renderer *renderer, TTF_Font *font) {
  // Set background color (dark blue)
  SDL_SetRenderDrawColor(renderer, 0, 0, 80, 255);
  SDL_RenderClear(renderer);
  
  // Title
  SDL_Color titleColor = {255, 255, 0, 255}; // Yellow
  renderText(renderer, font, "BREAKOUT", titleColor, SCREEN_WIDTH/2 - 70, 80);
  
  // Menu options
  SDL_Color selected = {255, 255, 255, 255}; // White
  SDL_Color unselected = {150, 150, 150, 255}; // Gray
  
  // Start Game option
  renderText(renderer, font, "Start Game", 
             (selectedOption == MENU_START) ? selected : unselected,
             SCREEN_WIDTH/2 - 70, 180);
  
  // Quit option
  renderText(renderer, font, "Quit", 
             (selectedOption == MENU_EXIT) ? selected : unselected,
             SCREEN_WIDTH/2 - 30, 220);
  
  // Instructions
  SDL_Color instructionColor = {200, 200, 200, 255}; // Light gray
  renderText(renderer, font, "Up/Down: Select, Enter: Confirm", 
             instructionColor, SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT - 60);
  
  // Display high score if exists
  renderHighScore(renderer, font, highScore);
}

// Render game over screen
void renderGameOverScreen(SDL_Renderer *renderer, TTF_Font *font, bool isWin) {
  // Set background color (dark red for loss, dark green for win)
  if (isWin) {
    SDL_SetRenderDrawColor(renderer, 0, 50, 0, 255); // Dark green
  } else {
    SDL_SetRenderDrawColor(renderer, 50, 0, 0, 255); // Dark red
  }
  SDL_RenderClear(renderer);
  
  // Game result message
  SDL_Color messageColor = isWin ? 
                          (SDL_Color){0, 255, 0, 255} :  // Bright green
                          (SDL_Color){255, 0, 0, 255};   // Bright red
  
  const char* message = isWin ? "YOU WIN!" : "GAME OVER";
  renderText(renderer, font, message, messageColor, 
            SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 40);
  
  // Score display
  char scoreText[32];
  sprintf(scoreText, "Your Score: %d", score);
  SDL_Color white = {255, 255, 255, 255};
  renderText(renderer, font, scoreText, white, SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2);
  
  // New high score notification
  if (score > highScore) {
    highScore = score;
    SDL_Color gold = {255, 215, 0, 255}; // Gold
    renderText(renderer, font, "NEW HIGH SCORE!", gold, 
              SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2 + 30);
  }
  
  // Instructions
  SDL_Color instructionColor = {200, 200, 200, 255}; // Light gray
  renderText(renderer, font, "Press R to restart or ESC for menu", 
            instructionColor, SCREEN_WIDTH/2 - 160, SCREEN_HEIGHT - 60);
}

// Reset game to initial state
void resetGame() {
  // Reset player
  player_X = SCREEN_WIDTH / 2 - 20;
  player_Y = SCREEN_HEIGHT - (20 * 3);
  player_vx = 0;
  player_vy = 0;
  
  // Reset ball
  ball_x = SCREEN_WIDTH / 2 - 10;
  ball_y = SCREEN_HEIGHT / 2 - 10;
  ball_vx = ballSpeed;
  ball_vy = ballSpeed;
  
  // Reset game variables
  score = 0;
  totalBall = 35;
  game_start = true;
  
  // Switch to playing state
  currentState = STATE_PLAYING;
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

  // Block list starts as NULL until game begins
  BlockNode *blockList = NULL;

  // Start in lobby state
  currentState = STATE_LOBBY;

  while (isRunning) {
    // Common game objects
    Rectangle playerBlock = {player_X, player_Y, 90, 20, {23, 231, 255, 255}};
    struct Arc ball = {ball_x, ball_y, 10, 0, M_PI * 2, {255, 0, 0, 255}};

    // Event handling - common for all states
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        isRunning = false;
      }

      // Handle key presses based on current state
      if (event.type == SDL_KEYDOWN) {
        // Global keys for any state
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          // ESC key returns to lobby from any state except lobby itself
          if (currentState != STATE_LOBBY) {
            // Free block list when returning to lobby
            BlockNode *current = blockList;
            while (current != NULL) {
              BlockNode *next = current->next;
              free(current);
              current = next;
            }
            blockList = NULL;
            currentState = STATE_LOBBY;
          }
        }

        // State-specific key handling
        switch (currentState) {
          case STATE_LOBBY:
            // Lobby navigation
            if (event.key.keysym.sym == SDLK_UP) {
              selectedOption = (selectedOption == MENU_START) ? MENU_EXIT : MENU_START;
            } else if (event.key.keysym.sym == SDLK_DOWN) {
              selectedOption = (selectedOption == MENU_EXIT) ? MENU_START : MENU_EXIT;
            } else if (event.key.keysym.sym == SDLK_RETURN || 
                      event.key.keysym.sym == SDLK_SPACE) {
              if (selectedOption == MENU_START) {
                // Start new game
                resetGame();
                blockList = createBlockList(totalBall);
              } else if (selectedOption == MENU_EXIT) {
                isRunning = false;
              }
            }
            break;

          case STATE_PLAYING:
            // Game controls
            if (event.key.keysym.sym == SDLK_a) {
              player_vx = -playerSpeed;
            } else if (event.key.keysym.sym == SDLK_d) {
              player_vx = playerSpeed;
            } else if (event.key.keysym.sym == SDLK_f && !event.key.repeat) {
              automatic_paddle = !automatic_paddle;
            }
            break;

          case STATE_GAME_OVER:
          case STATE_WIN:
            // Game over/win controls
            if (event.key.keysym.sym == SDLK_r) {
              // Restart game
              resetGame();
              
              // Free old block list
              BlockNode *current = blockList;
              while (current != NULL) {
                BlockNode *next = current->next;
                free(current);
                current = next;
              }
              
              // Create new block list
              blockList = createBlockList(totalBall);
            }
            break;

          default:
            break;
        }
      } else if (event.type == SDL_KEYUP) {
        // Handle key releases for gameplay
        if (currentState == STATE_PLAYING) {
          if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_d) {
            player_vx = 0;
          }
        }
      }
    }

    // State-specific updates and rendering
    switch (currentState) {
      case STATE_LOBBY:
        // Render lobby screen
        renderLobby(renderer, font);
        break;

      case STATE_PLAYING:
        // Update game state
        ball_x += ball_vx;
        ball_y += ball_vy;
        
        // Ball-paddle collision
        if (checkCollision(ball, playerBlock)) {
          ball_vy = -ballSpeed;
        }
        
        // Ball-wall collisions
        if (ball_x < 0) {
          ball_vx = ballSpeed;
        }
        if (ball_x > SCREEN_WIDTH - ball.r) {
          ball_vx = -ballSpeed;
        }
        if (ball_y < 0 + ball.r) {
          ball_vy = ballSpeed;
        }
        
        // Update player position
        player_X += player_vx;
        
        // Auto-paddle feature
        if (automatic_paddle) {
          player_X = ball_x - playerBlock.w/2; // Center paddle under ball
        }
        
        // Keep player within boundaries
        if (player_X < 0) {
          player_X = 0;
        }
        if (player_X > SCREEN_WIDTH - playerBlock.w) {
          player_X = SCREEN_WIDTH - playerBlock.w;
        }
        if (player_Y < 0) {
          player_Y = 0;
        }
        if (player_Y > SCREEN_HEIGHT - playerBlock.h) {
          player_Y = SCREEN_HEIGHT - playerBlock.h;
        }
        
        // Check for ball-block collisions
        breakBlock(ball, &blockList);
        
        // Check for win condition
        if (totalBall <= 0) {
          currentState = STATE_WIN;
        }
        
        // Check for lose condition (ball below screen)
        if (ball_y > SCREEN_HEIGHT) {
          currentState = STATE_GAME_OVER;
        }
        
        // Render game elements
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        drawRectangle(renderer, playerBlock);
        drawArc(renderer, ball);
        drawBlocks(renderer, blockList);
        renderScore(renderer, font, score);
        renderHighScore(renderer, font, highScore);
        break;

      case STATE_GAME_OVER:
        // Render game over screen
        renderGameOverScreen(renderer, font, false);
        break;

      case STATE_WIN:
        // Render win screen
        renderGameOverScreen(renderer, font, true);
        break;

      default:
        break;
    }

    // Present rendered frame
    SDL_RenderPresent(renderer);
    
    // Cap frame rate
    SDL_Delay(1000 / 60);
  }

  // Cleanup - outside the game loop
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
