#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h> // For sound effects
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 400

// Sound effects
#define MAX_SOUNDS 8

// Game states
typedef enum {
    STATE_LOBBY,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER,
    STATE_WIN,
    STATE_DIFFICULTY
} GameState;

// Menu options
typedef enum {
    MENU_START,
    MENU_DIFFICULTY,
    MENU_EXIT,
    MENU_TOTAL
} MenuOption;

// Difficulty levels
typedef enum {
    DIFFICULTY_EASY,
    DIFFICULTY_MEDIUM,
    DIFFICULTY_HARD,
    DIFFICULTY_TOTAL
} DifficultyLevel;

// Power-up types
typedef enum {
    POWER_NONE,
    POWER_WIDER_PADDLE,
    POWER_SLOWER_BALL,
    POWER_FASTER_BALL,
    POWER_MULTI_BALL,
    POWER_EXTRA_LIFE,
    POWER_TOTAL
} PowerUpType;

// Game state variables
GameState currentState = STATE_LOBBY;
MenuOption selectedOption = MENU_START;
int highScore = 0;
DifficultyLevel currentDifficulty = DIFFICULTY_MEDIUM;
DifficultyLevel selectedDifficulty = DIFFICULTY_MEDIUM;
int currentLevel = 1;
int lives = 3;

// Player variables
float player_X = SCREEN_WIDTH / 2 - 20;
float player_Y = SCREEN_HEIGHT - (20 * 3);
const float playerSpeed = 9.7;
float player_vy = 0;
float player_vx = 0;
int paddleWidth = 90; // Default paddle width
bool useMouse = false; // Option to use mouse control

// Ball variables
float ball_x = SCREEN_WIDTH / 2 - 10;
float ball_y = SCREEN_HEIGHT / 2 - 10;
const float normalBallSpeed = 5.0;
float ballSpeed = 5.0;
float ball_vx = 0.0;  // Will be set in resetGame()
float ball_vy = 0.0;  // Will be set in resetGame()
bool ballLaunched = false;

// Game variables
int totalBall = 35;
int score = 0;
bool game_start = false;
bool automatic_paddle = false;
bool paused = false;

// Rectangle structure (moved up before PowerUp)
typedef struct {
  int x, y, w, h;
  SDL_Color color; // property off RGBA which is stands for (RED, GREEN, BLUE
                   // AND ALPHA)
} Rectangle;

// Arc structure
struct Arc {
  int x, y;
  int r;
  double startAngle;
  double endAngle;
  SDL_Color color;
};

// Power-up variables
typedef struct PowerUp {
    PowerUpType type;
    Rectangle rect;
    bool active;
    int duration; // Frames remaining for active effects
    struct PowerUp* next;
} PowerUp;

PowerUp* activePowerUps = NULL;
PowerUp* fallingPowerUps = NULL;

// Sound variables
Mix_Chunk* sounds[MAX_SOUNDS];
bool sound_enabled = false; // Flag to track if sound is available

enum SoundEffects {
    SOUND_PADDLE_HIT,
    SOUND_BLOCK_HIT,
    SOUND_POWER_UP,
    SOUND_LEVEL_COMPLETE,
    SOUND_GAME_OVER,
    SOUND_MENU_SELECT,
    SOUND_MENU_CLICK
};

// Sound file paths
const char* sound_files[MAX_SOUNDS] = {
    "sounds/paddle_hit.wav",
    "sounds/block_hit.wav",
    "sounds/power_up.wav",
    "sounds/level_complete.wav",
    "sounds/game_over.wav",
    "sounds/menu_select.wav",
    "sounds/menu_click.wav"
};

// Rectangle and Arc structures moved up before PowerUp structure

typedef struct BlockNode {
  Rectangle block;
  int health;        // Block health - how many hits it takes to destroy
  int scoreValue;    // Score value when destroyed
  bool dropsPowerUp; // Whether this block drops a power-up when destroyed
  PowerUpType powerUpType; // Type of power-up to drop
  struct BlockNode *next;
} BlockNode;

// Function prototypes
void drawRectangle(SDL_Renderer *renderer, Rectangle rectangle);

double degreesToRadians(double degrees) { return degrees * M_PI / 180.0; }

// Initialize power-up system
void initPowerUps() {
    // Free any existing power-ups
    PowerUp* current = activePowerUps;
    while (current != NULL) {
        PowerUp* temp = current;
        current = current->next;
        free(temp);
    }
    activePowerUps = NULL;
    
    current = fallingPowerUps;
    while (current != NULL) {
        PowerUp* temp = current;
        current = current->next;
        free(temp);
    }
    fallingPowerUps = NULL;
}

// Create a new power-up
PowerUp* createPowerUp(int x, int y, PowerUpType type) {
    PowerUp* powerUp = (PowerUp*)malloc(sizeof(PowerUp));
    if (powerUp == NULL) {
        fprintf(stderr, "Failed to allocate memory for power-up\n");
        return NULL;
    }
    
    powerUp->type = type;
    powerUp->rect.x = x;
    powerUp->rect.y = y;
    powerUp->rect.w = 20;
    powerUp->rect.h = 20;
    
    // Set color based on power-up type
    switch (type) {
        case POWER_WIDER_PADDLE:
            powerUp->rect.color = (SDL_Color){255, 255, 0, 255}; // Yellow
            break;
        case POWER_SLOWER_BALL:
            powerUp->rect.color = (SDL_Color){0, 255, 0, 255}; // Green
            break;
        case POWER_FASTER_BALL:
            powerUp->rect.color = (SDL_Color){255, 0, 0, 255}; // Red
            break;
        case POWER_MULTI_BALL:
            powerUp->rect.color = (SDL_Color){0, 0, 255, 255}; // Blue
            break;
        case POWER_EXTRA_LIFE:
            powerUp->rect.color = (SDL_Color){255, 0, 255, 255}; // Magenta
            break;
        default:
            powerUp->rect.color = (SDL_Color){255, 255, 255, 255}; // White
            break;
    }
    
    powerUp->active = false;
    powerUp->duration = 0;
    powerUp->next = NULL;
    
    return powerUp;
}

// Add a power-up to the falling list
void addFallingPowerUp(int x, int y, PowerUpType type) {
    PowerUp* powerUp = createPowerUp(x, y, type);
    if (powerUp == NULL) return;
    
    // Add to falling power-ups list
    powerUp->next = fallingPowerUps;
    fallingPowerUps = powerUp;
}

// Activate a power-up effect
void activatePowerUp(PowerUpType type) {
    PowerUp* powerUp = (PowerUp*)malloc(sizeof(PowerUp));
    if (powerUp == NULL) {
        fprintf(stderr, "Failed to allocate memory for power-up\n");
        return;
    }
    
    powerUp->type = type;
    powerUp->active = true;
    powerUp->duration = 600; // 10 seconds at 60 FPS
    powerUp->next = activePowerUps;
    activePowerUps = powerUp;
    
    // Apply immediate effect based on type
    switch (type) {
        case POWER_WIDER_PADDLE:
            paddleWidth = 150; // Wider paddle
            break;
        case POWER_SLOWER_BALL:
            ballSpeed = normalBallSpeed * 0.7f; // Slower ball
            ball_vx = (ball_vx > 0) ? ballSpeed : -ballSpeed;
            ball_vy = (ball_vy > 0) ? ballSpeed : -ballSpeed;
            break;
        case POWER_FASTER_BALL:
            ballSpeed = normalBallSpeed * 1.5f; // Faster ball
            ball_vx = (ball_vx > 0) ? ballSpeed : -ballSpeed;
            ball_vy = (ball_vy > 0) ? ballSpeed : -ballSpeed;
            break;
        case POWER_MULTI_BALL:
            // TODO: Implement multiple balls
            break;
        case POWER_EXTRA_LIFE:
            lives++; // Add an extra life
            break;
        default:
            break;
    }
    
    // Play power-up sound if sound is enabled
    if (sound_enabled && sounds[SOUND_POWER_UP] != NULL) {
        Mix_PlayChannel(-1, sounds[SOUND_POWER_UP], 0);
    }
}

// Update active power-ups (decrease duration, remove expired)
void updatePowerUps() {
    PowerUp* current = activePowerUps;
    PowerUp* prev = NULL;
    
    while (current != NULL) {
        current->duration--;
        
        if (current->duration <= 0) {
            // Reset effects when power-up expires
            switch (current->type) {
                case POWER_WIDER_PADDLE:
                    paddleWidth = 90; // Reset paddle width
                    break;
                case POWER_SLOWER_BALL:
                case POWER_FASTER_BALL:
                    ballSpeed = normalBallSpeed; // Reset ball speed
                    ball_vx = (ball_vx > 0) ? ballSpeed : -ballSpeed;
                    ball_vy = (ball_vy > 0) ? ballSpeed : -ballSpeed;
                    break;
                default:
                    break;
            }
            
            // Remove from list
            PowerUp* toRemove = current;
            if (prev == NULL) {
                activePowerUps = current->next;
            } else {
                prev->next = current->next;
            }
            current = current->next;
            free(toRemove);
        } else {
            prev = current;
            current = current->next;
        }
    }
}

// Update falling power-ups (move down, check for collection)
void updateFallingPowerUps(Rectangle playerBlock) {
    PowerUp* current = fallingPowerUps;
    PowerUp* prev = NULL;
    
    while (current != NULL) {
        // Move power-up down
        current->rect.y += 2;
        
        // Check if player collected the power-up
        if (current->rect.y + current->rect.h >= playerBlock.y &&
            current->rect.y <= playerBlock.y + playerBlock.h &&
            current->rect.x + current->rect.w >= playerBlock.x &&
            current->rect.x <= playerBlock.x + playerBlock.w) {
            
            // Activate the power-up
            activatePowerUp(current->type);
            
            // Remove from falling list
            PowerUp* toRemove = current;
            if (prev == NULL) {
                fallingPowerUps = current->next;
            } else {
                prev->next = current->next;
            }
            current = current->next;
            free(toRemove);
        }
        // Check if power-up is out of screen
        else if (current->rect.y > SCREEN_HEIGHT) {
            PowerUp* toRemove = current;
            if (prev == NULL) {
                fallingPowerUps = current->next;
            } else {
                prev->next = current->next;
            }
            current = current->next;
            free(toRemove);
        } else {
            prev = current;
            current = current->next;
        }
    }
}

// Render falling power-ups
void renderFallingPowerUps(SDL_Renderer* renderer) {
    PowerUp* current = fallingPowerUps;
    
    while (current != NULL) {
        drawRectangle(renderer, current->rect);
        current = current->next;
    }
}

// Create blocks with different health based on difficulty
BlockNode *createBlockList(int size) {
  BlockNode *head = NULL;
  BlockNode *tail = NULL;

  int blockWidth = 50;
  int blockHeight = 20;
  int padding = 10;
  int blocksPerRow = SCREEN_WIDTH / (blockWidth + padding);
  
  // Seed random number generator if not done already
  static bool seeded = false;
  if (!seeded) {
    srand(time(NULL));
    seeded = true;
  }

  for (int i = 0; i < size; i++) {
    BlockNode *newNode = (BlockNode *)malloc(sizeof(BlockNode));
    newNode->block.x = (i % blocksPerRow) * (blockWidth + padding) + 5;
    newNode->block.y = (i / blocksPerRow) * (blockHeight + padding);
    newNode->block.w = blockWidth;
    newNode->block.h = blockHeight;
    
    // Determine block health based on row and difficulty
    int row = i / blocksPerRow;
    
    // More health for blocks at the top
    if (row < 2) {
        // Top rows have higher health on higher difficulties
        switch(currentDifficulty) {
            case DIFFICULTY_EASY:
                newNode->health = 1;
                break;
            case DIFFICULTY_MEDIUM:
                newNode->health = row == 0 ? 2 : 1;
                break;
            case DIFFICULTY_HARD:
                newNode->health = row == 0 ? 3 : (row == 1 ? 2 : 1);
                break;
            default:
                newNode->health = 1;
        }
    } else {
        newNode->health = 1;
    }
    
    // Set score value based on health
    newNode->scoreValue = newNode->health * 10;
    
    // Determine color based on health
    switch (newNode->health) {
        case 1:
            newNode->block.color.r = 0;
            newNode->block.color.g = 255;
            newNode->block.color.b = 0;
            break;
        case 2:
            newNode->block.color.r = 255;
            newNode->block.color.g = 255;
            newNode->block.color.b = 0;
            break;
        case 3:
            newNode->block.color.r = 255;
            newNode->block.color.g = 0;
            newNode->block.color.b = 0;
            break;
        default:
            newNode->block.color.r = rand() % 256;
            newNode->block.color.g = rand() % 256;
            newNode->block.color.b = rand() % 256;
    }
    newNode->block.color.a = 255;
    
    // Random chance to drop a power-up (10%)
    if (rand() % 10 == 0) {
        newNode->dropsPowerUp = true;
        newNode->powerUpType = (PowerUpType)(rand() % (POWER_TOTAL - 1) + 1);
    } else {
        newNode->dropsPowerUp = false;
        newNode->powerUpType = POWER_NONE;
    }
    
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

// Handle block collision with updated health system
void breakBlock(struct Arc ball, BlockNode **head) {
  BlockNode *current = *head;
  BlockNode *prev = NULL;

  while (current != NULL) {
    Rectangle blk = current->block;
    // Simple AABB (Axis-Aligned Bounding Box) collision
    if (ball.x + ball.r > blk.x && ball.x - ball.r < blk.x + blk.w &&
        ball.y + ball.r > blk.y && ball.y - ball.r < blk.y + blk.h) {

      // Collision detected - change ball direction
      // Determine if hit was from top/bottom or sides
      float overlapLeft = ball.x + ball.r - blk.x;
      float overlapRight = blk.x + blk.w - (ball.x - ball.r);
      float overlapTop = ball.y + ball.r - blk.y;
      float overlapBottom = blk.y + blk.h - (ball.y - ball.r);

      // Find smallest overlap to determine direction
      float minOverlapX = (overlapLeft < overlapRight) ? overlapLeft : overlapRight;
      float minOverlapY = (overlapTop < overlapBottom) ? overlapTop : overlapBottom;

      // Change ball direction based on collision side
      if (minOverlapX < minOverlapY) {
        // Hit from left or right
        ball_vx = -ball_vx;
      } else {
        // Hit from top or bottom
        ball_vy = -ball_vy;
      }
      
      // Play block hit sound if sound is enabled
      if (sound_enabled && sounds[SOUND_BLOCK_HIT] != NULL) {
          Mix_PlayChannel(-1, sounds[SOUND_BLOCK_HIT], 0);
      }

      // Decrease block health
      current->health--;
      
      // Update block color based on remaining health
      switch (current->health) {
        case 2:
          current->block.color.r = 255;
          current->block.color.g = 255;
          current->block.color.b = 0;
          break;
        case 1:
          current->block.color.r = 0;
          current->block.color.g = 255;
          current->block.color.b = 0;
          break;
      }
      
      // Only remove block if health depleted
      if (current->health <= 0) {
        // Add score
        score += current->scoreValue;
        totalBall--;
        
        // Check if block drops a power-up
        if (current->dropsPowerUp) {
          addFallingPowerUp(current->block.x + current->block.w/2, 
                           current->block.y + current->block.h/2,
                           current->powerUpType);
        }
        
        // Remove the block
        if (prev == NULL) {
          *head = current->next;
        } else {
          prev->next = current->next;
        }
        BlockNode *temp = current;
        current = current->next;
        free(temp);
      } else {
        prev = current;
        current = current->next;
      }
      
      return; // Process one collision per frame
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

// Render difficulty selection screen
void renderDifficultyScreen(SDL_Renderer *renderer, TTF_Font *font) {
  // Set background color (dark blue)
  SDL_SetRenderDrawColor(renderer, 0, 0, 100, 255);
  SDL_RenderClear(renderer);
  
  // Title
  SDL_Color titleColor = {255, 255, 0, 255}; // Yellow
  renderText(renderer, font, "SELECT DIFFICULTY", titleColor, SCREEN_WIDTH/2 - 120, 80);
  
  // Difficulty options
  SDL_Color selected = {255, 255, 255, 255}; // White
  SDL_Color unselected = {150, 150, 150, 255}; // Gray
  
  // Easy option
  renderText(renderer, font, "Easy", 
             (selectedDifficulty == DIFFICULTY_EASY) ? selected : unselected,
             SCREEN_WIDTH/2 - 30, 160);
  
  // Medium option
  renderText(renderer, font, "Medium", 
             (selectedDifficulty == DIFFICULTY_MEDIUM) ? selected : unselected,
             SCREEN_WIDTH/2 - 50, 200);
  
  // Hard option
  renderText(renderer, font, "Hard", 
             (selectedDifficulty == DIFFICULTY_HARD) ? selected : unselected,
             SCREEN_WIDTH/2 - 30, 240);
  
  // Instructions
  SDL_Color instructionColor = {200, 200, 200, 255}; // Light gray
  renderText(renderer, font, "Up/Down: Select, Enter: Confirm, ESC: Back", 
             instructionColor, SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT - 60);
}

// Render the lobby screen with updated options
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
             SCREEN_WIDTH/2 - 70, 160);
  
  // Difficulty option
  renderText(renderer, font, "Difficulty", 
             (selectedOption == MENU_DIFFICULTY) ? selected : unselected,
             SCREEN_WIDTH/2 - 60, 200);
  
  // Quit option
  renderText(renderer, font, "Quit", 
             (selectedOption == MENU_EXIT) ? selected : unselected,
             SCREEN_WIDTH/2 - 30, 240);
  
  // Current difficulty display
  char diffText[20];
  switch(currentDifficulty) {
    case DIFFICULTY_EASY:
      sprintf(diffText, "Difficulty: Easy");
      break;
    case DIFFICULTY_MEDIUM:
      sprintf(diffText, "Difficulty: Medium");
      break;
    case DIFFICULTY_HARD:
      sprintf(diffText, "Difficulty: Hard");
      break;
    case DIFFICULTY_TOTAL:
      // This should never happen, but needed for exhaustive switch
      sprintf(diffText, "Difficulty: Unknown");
      break;
  }
  renderText(renderer, font, diffText, unselected, SCREEN_WIDTH/2 - 80, 280);
  
  // Instructions
  SDL_Color instructionColor = {200, 200, 200, 255}; // Light gray
  renderText(renderer, font, "Up/Down: Select, Enter: Confirm", 
             instructionColor, SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT - 60);
  
  // Display high score if exists
  renderHighScore(renderer, font, highScore);
}

// Render pause screen overlay
void renderPauseScreen(SDL_Renderer *renderer, TTF_Font *font) {
  // Semi-transparent overlay
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
  SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_RenderFillRect(renderer, &overlay);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
  
  // Pause message
  SDL_Color white = {255, 255, 255, 255};
  renderText(renderer, font, "PAUSED", white, SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT/2 - 30);
  
  // Instructions
  SDL_Color instructionColor = {200, 200, 200, 255};
  renderText(renderer, font, "Press P to Resume, ESC for Menu", 
             instructionColor, SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 20);
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

// Load and initialize sound effects
bool initSounds() {
    // Initialize all sound pointers to NULL
    for (int i = 0; i < MAX_SOUNDS; i++) {
        sounds[i] = NULL;
    }
    
    // Try to initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        printf("Sound will be disabled.\n");
        sound_enabled = false;
        return false;
    }
    
    // Load sound effects
    bool all_sounds_loaded = true;
    int sounds_loaded = 0;
    
    for (int i = 0; i < MAX_SOUNDS; i++) {
        sounds[i] = Mix_LoadWAV(sound_files[i]);
        if (sounds[i] == NULL) {
            printf("Warning: Could not load sound file: %s\n", sound_files[i]);
            all_sounds_loaded = false;
        } else {
            sounds_loaded++;
        }
    }
    
    // Enable sound if at least some sounds were loaded
    sound_enabled = (sounds_loaded > 0);
    
    if (sounds_loaded == 0) {
        printf("No sound files could be loaded. Sound will be disabled.\n");
        printf("To enable sound, place WAV files in the 'sounds' directory.\n");
        return false;
    } else if (!all_sounds_loaded) {
        printf("Some sound files were missing. Partial sound support enabled.\n");
        printf("Loaded %d of %d sound files.\n", sounds_loaded, MAX_SOUNDS);
    } else {
        printf("All sound files loaded successfully.\n");
    }
    
    return sound_enabled;
}

// Clean up sound resources
void cleanupSounds() {
    for (int i = 0; i < MAX_SOUNDS; i++) {
        if (sounds[i] != NULL) {
            Mix_FreeChunk(sounds[i]);
            sounds[i] = NULL;
        }
    }
    
    Mix_CloseAudio();
}

// Apply difficulty settings
void applyDifficultySettings() {
    switch(currentDifficulty) {
        case DIFFICULTY_EASY:
            lives = 5;
            ballSpeed = normalBallSpeed * 0.8f;
            break;
        case DIFFICULTY_MEDIUM:
            lives = 3;
            ballSpeed = normalBallSpeed;
            break;
        case DIFFICULTY_HARD:
            lives = 2;
            ballSpeed = normalBallSpeed * 1.2f;
            break;
        case DIFFICULTY_TOTAL:
            // This should never happen, but needed for exhaustive switch
            lives = 3;
            ballSpeed = normalBallSpeed;
            break;
    }
    
    // Update ball velocity
    ball_vx = ballSpeed;
    ball_vy = ballSpeed;
}

// Initialize level with appropriate number of blocks and layout
void initializeLevel(int level) {
    // Adjust difficulty based on level
    int blockCount = 35 + (level - 1) * 5; // More blocks in higher levels
    blockCount = (blockCount > 70) ? 70 : blockCount; // Cap at 70 blocks
    
    totalBall = blockCount;
    currentLevel = level;
    
    // Reset power-ups
    initPowerUps();
    
    // Reset paddle
    paddleWidth = 90;
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
  
  // Apply difficulty settings
  applyDifficultySettings();
  
  // Reset game variables
  score = 0;
  ballLaunched = false;
  paused = false;
  
  // Initialize ball velocity (moved from global initialization)
  ball_vx = ballSpeed;
  ball_vy = ballSpeed;
  
  // Initialize level 1
  initializeLevel(1);
  
  // Initialize power-ups
  initPowerUps();
  
  // Switch to playing state
  currentState = STATE_PLAYING;
}

// Create visual effects for collisions
void createCollisionEffect(int x, int y, SDL_Color color) {
    // Placeholder for particle effects
    // This would normally create particles, but for simplicity we'll leave it empty
}

int main() {
  // Seed random number generator
  srand(time(NULL));

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }

  if (TTF_Init() == -1) {
    printf("TTF_Init: %s\n", TTF_GetError());
    return -1;
  }
  
  // Initialize SDL_mixer and load sounds
  if (!initSounds()) {
    printf("Warning: Sound initialization failed. Continuing without sound.\n");
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
              // Play menu selection sound if sound is enabled
              if (sound_enabled && sounds[SOUND_MENU_SELECT] != NULL) {
                  Mix_PlayChannel(-1, sounds[SOUND_MENU_SELECT], 0);
              }
              
              // Move selection up
              selectedOption = (selectedOption == MENU_START) ? MENU_EXIT : 
                              (selectedOption == MENU_EXIT) ? MENU_DIFFICULTY : MENU_START;
            } else if (event.key.keysym.sym == SDLK_DOWN) {
              // Play menu selection sound
              Mix_PlayChannel(-1, sounds[SOUND_MENU_SELECT], 0);
              
              // Move selection down
              selectedOption = (selectedOption == MENU_START) ? MENU_DIFFICULTY : 
                              (selectedOption == MENU_DIFFICULTY) ? MENU_EXIT : MENU_START;
            } else if (event.key.keysym.sym == SDLK_RETURN || 
                      event.key.keysym.sym == SDLK_SPACE) {
              // Play menu click sound if sound is enabled
              if (sound_enabled && sounds[SOUND_MENU_CLICK] != NULL) {
                  Mix_PlayChannel(-1, sounds[SOUND_MENU_CLICK], 0);
              }
              
              if (selectedOption == MENU_START) {
                // Start new game
                resetGame();
                blockList = createBlockList(totalBall);
              } else if (selectedOption == MENU_DIFFICULTY) {
                // Go to difficulty selection screen
                currentState = STATE_DIFFICULTY;
                selectedDifficulty = currentDifficulty;
              } else if (selectedOption == MENU_EXIT) {
                isRunning = false;
              }
            }
            break;
            
          case STATE_DIFFICULTY:
            // Difficulty selection navigation
            if (event.key.keysym.sym == SDLK_UP) {
              // Play menu selection sound
              Mix_PlayChannel(-1, sounds[SOUND_MENU_SELECT], 0);
              
              // Move selection up
              selectedDifficulty = (selectedDifficulty == DIFFICULTY_EASY) ? DIFFICULTY_HARD : 
                                  (selectedDifficulty == DIFFICULTY_MEDIUM) ? DIFFICULTY_EASY : DIFFICULTY_MEDIUM;
            } else if (event.key.keysym.sym == SDLK_DOWN) {
              // Play menu selection sound
              Mix_PlayChannel(-1, sounds[SOUND_MENU_SELECT], 0);
              
              // Move selection down
              selectedDifficulty = (selectedDifficulty == DIFFICULTY_EASY) ? DIFFICULTY_MEDIUM : 
                                  (selectedDifficulty == DIFFICULTY_MEDIUM) ? DIFFICULTY_HARD : DIFFICULTY_EASY;
            } else if (event.key.keysym.sym == SDLK_RETURN || 
                      event.key.keysym.sym == SDLK_SPACE) {
              // Play menu click sound
              Mix_PlayChannel(-1, sounds[SOUND_MENU_CLICK], 0);
              
              // Set difficulty and return to main menu
              currentDifficulty = selectedDifficulty;
              currentState = STATE_LOBBY;
            } else if (event.key.keysym.sym == SDLK_ESCAPE) {
              // Return to main menu without changing difficulty
              currentState = STATE_LOBBY;
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
            } else if (event.key.keysym.sym == SDLK_m && !event.key.repeat) {
              // Toggle mouse control
              useMouse = !useMouse;
            } else if (event.key.keysym.sym == SDLK_SPACE) {
              // Launch the ball if not launched yet
              if (!ballLaunched) {
                ballLaunched = true;
                
                // Set initial ball velocity
                ball_vx = ballSpeed;
                ball_vy = -ballSpeed; // Start going up
              }
            } else if (event.key.keysym.sym == SDLK_p) {
              // Toggle pause
              paused = !paused;
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
        if (!paused) {
          // Update power-ups first
          updatePowerUps();
          updateFallingPowerUps(playerBlock);
          
          // Mouse control
          if (useMouse) {
            int mouseX;
            SDL_GetMouseState(&mouseX, NULL);
            player_X = mouseX - (paddleWidth / 2);
          }
          
          // If ball hasn't been launched, keep it on paddle
          if (!ballLaunched) {
            ball_x = player_X + (paddleWidth / 2);
            ball_y = player_Y - 15;
          } else {
            // Update ball position
            ball_x += ball_vx;
            ball_y += ball_vy;
          }
          
          // Ball-paddle collision
          if (checkCollision(ball, playerBlock) && ball_vy > 0) { // Only collide when ball moving down
            // Play paddle hit sound if sound is enabled
            if (sound_enabled && sounds[SOUND_PADDLE_HIT] != NULL) {
                Mix_PlayChannel(-1, sounds[SOUND_PADDLE_HIT], 0);
            }
            
            // Bounce ball
            ball_vy = -ballSpeed;
            
            // Angle based on where the ball hits the paddle
            float hitPosition = (ball.x - player_X) / paddleWidth;
            ball_vx = ballSpeed * (hitPosition - 0.5f) * 2; // -ballSpeed to +ballSpeed
            
            // Create visual effect
            createCollisionEffect(ball.x, ball.y, (SDL_Color){100, 100, 255, 255});
          }
          
          // Ball-wall collisions
          if (ball_x < 0 + ball.r) {
            ball_vx = fabs(ball_vx); // Ensure positive (moving right)
            createCollisionEffect(ball.x, ball.y, (SDL_Color){255, 100, 100, 255});
          }
          if (ball_x > SCREEN_WIDTH - ball.r) {
            ball_vx = -fabs(ball_vx); // Ensure negative (moving left)
            createCollisionEffect(ball.x, ball.y, (SDL_Color){255, 100, 100, 255});
          }
          if (ball_y < 0 + ball.r) {
            ball_vy = fabs(ball_vy); // Ensure positive (moving down)
            createCollisionEffect(ball.x, ball.y, (SDL_Color){255, 100, 100, 255});
          }
          
          // Update player position
          player_X += player_vx;
          
          // Auto-paddle feature
          if (automatic_paddle && ballLaunched) {
            player_X = ball_x - (paddleWidth / 2); // Center paddle under ball
          }
          
          // Keep player within boundaries
          if (player_X < 0) {
            player_X = 0;
          }
          if (player_X > SCREEN_WIDTH - paddleWidth) {
            player_X = SCREEN_WIDTH - paddleWidth;
          }
          
          // Check for ball-block collisions
          if (ballLaunched) {
            breakBlock(ball, &blockList);
          }
          
          // Check for win condition
          if (totalBall <= 0) {
            // Play level complete sound if sound is enabled
            if (sound_enabled && sounds[SOUND_LEVEL_COMPLETE] != NULL) {
                Mix_PlayChannel(-1, sounds[SOUND_LEVEL_COMPLETE], 0);
            }
            
            // For now, just go to win screen
            currentState = STATE_WIN;
            
            // TODO: Implement level progression
            // currentLevel++;
            // initializeLevel(currentLevel);
            // blockList = createBlockList(totalBall);
            // ballLaunched = false;
          }
          
          // Check for lose condition (ball below screen)
          if (ball_y > SCREEN_HEIGHT) {
            lives--;
            
            if (lives <= 0) {
              // Game over sound if sound is enabled
              if (sound_enabled && sounds[SOUND_GAME_OVER] != NULL) {
                  Mix_PlayChannel(-1, sounds[SOUND_GAME_OVER], 0);
              }
              currentState = STATE_GAME_OVER;
            } else {
              // Reset ball but continue game
              ball_x = player_X + (paddleWidth / 2);
              ball_y = player_Y - 15;
              ballLaunched = false;
            }
          }
        }
        
        // Render game elements
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // Update player block with correct width
        playerBlock.w = paddleWidth;
        
        // Draw game elements
        drawRectangle(renderer, playerBlock);
        drawArc(renderer, ball);
        drawBlocks(renderer, blockList);
        renderFallingPowerUps(renderer);
        
        // Display HUD
        renderScore(renderer, font, score);
        renderHighScore(renderer, font, highScore);
        
        // Display lives
        char livesText[20];
        sprintf(livesText, "Lives: %d", lives);
        SDL_Color livesColor = {255, 255, 255, 255};
        renderText(renderer, font, livesText, livesColor, 10, 10);
        
        // Display level
        char levelText[20];
        sprintf(levelText, "Level: %d", currentLevel);
        renderText(renderer, font, levelText, livesColor, SCREEN_WIDTH - 100, 10);
        
        // Display launch instruction if ball not launched
        if (!ballLaunched) {
          renderText(renderer, font, "Press SPACE to launch", 
                    (SDL_Color){200, 200, 200, 255}, SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT - 60);
        }
        
        // Display control info
        if (useMouse) {
          renderText(renderer, font, "Mouse Control: ON", 
                    (SDL_Color){150, 150, 255, 255}, 10, 40);
        }
        if (automatic_paddle) {
          renderText(renderer, font, "Auto Paddle: ON", 
                    (SDL_Color){150, 255, 150, 255}, 10, 70);
        }
        
        // Render pause overlay if paused
        if (paused) {
          renderPauseScreen(renderer, font);
        }
        break;
        
      case STATE_DIFFICULTY:
        // Render difficulty selection screen
        renderDifficultyScreen(renderer, font);
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
  
  // Free power-ups
  PowerUp* powerUp = activePowerUps;
  while (powerUp != NULL) {
    PowerUp* next = powerUp->next;
    free(powerUp);
    powerUp = next;
  }
  
  powerUp = fallingPowerUps;
  while (powerUp != NULL) {
    PowerUp* next = powerUp->next;
    free(powerUp);
    powerUp = next;
  }

  // Clean up SDL resources
  cleanupSounds();
  TTF_CloseFont(font);
  TTF_Quit();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  printf("Goodbye World!\n");

  return 0;
}
