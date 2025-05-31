#include <SDL.h>
#include <SDL_ttf.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Constants
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 400
#define GRID_SIZE 20     // Size of each snake segment and grid cell
#define INITIAL_LENGTH 3 // Initial snake length
#define GAME_SPEED 10    // Frames per second

// Direction enumeration
typedef enum { UP, RIGHT, DOWN, LEFT } Direction;

// Structure for snake segment (node in doubly linked list)
typedef struct SnakeSegment {
  int x, y;                  // Position of segment
  SDL_Color color;           // Color of segment
  struct SnakeSegment *prev; // Pointer to previous segment
  struct SnakeSegment *next; // Pointer to next segment
} SnakeSegment;

// Structure for food
typedef struct {
  int x, y;
  SDL_Color color;
} Food;

// Global variables
SnakeSegment *snake_head = NULL;     // Head of the snake
SnakeSegment *snake_tail = NULL;     // Tail of the snake
Food food;                           // Food item
Direction current_direction = RIGHT; // Initial direction
Direction next_direction = RIGHT;    // Next direction (for buffering input)
int score = 0;                       // Player's score
bool game_over = false;              // Game over flag
bool should_grow = false;            // Flag to indicate if snake should grow

// Function prototypes
void initializeGame();
void cleanupGame();
SnakeSegment *createSegment(int x, int y, SDL_Color color);
void insertHead(int x, int y);
void deleteTail();
void updatePositions();
void generateFood();
// bool checkCollisionWithSelf();
bool checkCollisionWithWall();
bool checkCollisionWithFood();
void renderSnake(SDL_Renderer *renderer);
void renderFood(SDL_Renderer *renderer);
void renderScore(SDL_Renderer *renderer, TTF_Font *font);
void renderGameOver(SDL_Renderer *renderer, TTF_Font *font);

// Initialize the snake with initial segments
void initializeGame() {
  // Seed random number generator
  srand(time(NULL));

  // Clear any existing snake
  SnakeSegment *current = snake_head;
  while (current != NULL) {
    SnakeSegment *temp = current;
    current = current->next;
    free(temp);
  }

  snake_head = NULL;
  snake_tail = NULL;

  // Create initial snake segments
  SDL_Color head_color = {0, 255, 0, 255}; // Green for head
  SDL_Color body_color = {0, 200, 0, 255}; // Darker green for body

  // Create initial snake at the center of the screen, heading right
  int start_x = (SCREEN_WIDTH / GRID_SIZE / 2) * GRID_SIZE;
  int start_y = (SCREEN_HEIGHT / GRID_SIZE / 2) * GRID_SIZE;

  // Create head
  snake_head = createSegment(start_x, start_y, head_color);
  snake_tail = snake_head;

  // Create body segments
  for (int i = 1; i < INITIAL_LENGTH; i++) {
    // Add segments to the left of the head
    SnakeSegment *new_segment =
        createSegment(start_x - i * GRID_SIZE, start_y, body_color);
    new_segment->next = snake_head;
    snake_head->prev = new_segment;
    snake_head = new_segment;
  }

  // Reset game state
  current_direction = RIGHT;
  next_direction = RIGHT;
  score = 0;
  game_over = false;
  should_grow = false;

  // Generate initial food
  generateFood();
}

// Clean up the snake linked list
void cleanupGame() {
  SnakeSegment *current = snake_head;
  while (current != NULL) {
    SnakeSegment *temp = current;
    current = current->next;
    free(temp);
  }
  snake_head = NULL;
  snake_tail = NULL;
}

// Create a new snake segment
SnakeSegment *createSegment(int x, int y, SDL_Color color) {
  SnakeSegment *segment = (SnakeSegment *)malloc(sizeof(SnakeSegment));
  if (segment == NULL) {
    fprintf(stderr, "Failed to allocate memory for snake segment\n");
    exit(1);
  }

  segment->x = x;
  segment->y = y;
  segment->color = color;
  segment->prev = NULL;
  segment->next = NULL;

  return segment;
}

// Insert a new segment at the head of the snake
void insertHead(int x, int y) {
  SDL_Color head_color = {0, 255, 0, 255}; // Green for head
  SDL_Color body_color = {0, 200, 0, 255}; // Darker green for body

  // Change current head color to body color
  if (snake_head != NULL) {
    snake_head->color = body_color;
  }

  // Create new head
  SnakeSegment *new_head = createSegment(x, y, head_color);

  // Insert at the beginning of the list
  if (snake_head == NULL) {
    snake_head = new_head;
    snake_tail = new_head;
  } else {
    new_head->next = snake_head;
    snake_head->prev = new_head;
    snake_head = new_head;
  }
}

// Delete the tail segment
void deleteTail() {
  if (snake_tail == NULL) {
    return;
  }

  if (snake_head == snake_tail) {
    // Only one segment
    free(snake_tail);
    snake_head = NULL;
    snake_tail = NULL;
    return;
  }

  // Multiple segments
  SnakeSegment *new_tail = snake_tail->prev;
  new_tail->next = NULL;
  free(snake_tail);
  snake_tail = new_tail;
}

// Update the positions of all snake segments
void updatePositions() {
  if (game_over) {
    return;
  }

  // Calculate new head position based on current direction
  int new_x = snake_head->x;
  int new_y = snake_head->y;

  switch (current_direction) {
  case UP:
    new_y -= GRID_SIZE;
    break;
  case RIGHT:
    new_x += GRID_SIZE;
    break;
  case DOWN:
    new_y += GRID_SIZE;
    break;
  case LEFT:
    new_x -= GRID_SIZE;
    break;
  }

  // Insert new head
  insertHead(new_x, new_y);

  // Delete tail if not growing
  if (!should_grow) {
    deleteTail();
  } else {
    should_grow = false; // Reset growth flag
  }
}

// Generate a new food item at a random position
void generateFood() {
  // Generate random position
  int max_x = SCREEN_WIDTH / GRID_SIZE - 1;
  int max_y = SCREEN_HEIGHT / GRID_SIZE - 1;

  // Try to place food in a position not occupied by the snake
  bool valid_position = false;
  int x, y;

  while (!valid_position) {
    valid_position = true;
    x = (rand() % max_x) * GRID_SIZE;
    y = (rand() % max_y) * GRID_SIZE;

    // Check if this position collides with snake
    SnakeSegment *current = snake_head;
    while (current != NULL) {
      if (current->x == x && current->y == y) {
        valid_position = false;
        break;
      }
      current = current->next;
    }
  }

  food.x = x;
  food.y = y;
  food.color = (SDL_Color){255, 0, 0, 255}; // Red color for food
}

// Check if snake head collides with its own body
// bool checkCollisionWithSelf() {
//     SnakeSegment *current = snake_head->next; // Start with the segment after
//     the head
//
//     while (current != NULL) {
//         if (snake_head->x == current->x && snake_head->y == current->y) {
//             return true; // Collision detected
//         }
//         current = current->next;
//     }
//
//     return false; // No collision
// }

// Check if snake head collides with wall
bool checkCollisionWithWall() {
  return (snake_head->x < 0 || snake_head->x >= SCREEN_WIDTH ||
          snake_head->y < 0 || snake_head->y >= SCREEN_HEIGHT);
}

// Check if snake head collides with food
bool checkCollisionWithFood() {
  return (snake_head->x == food.x && snake_head->y == food.y);
}

// Render the snake
void renderSnake(SDL_Renderer *renderer) {
  SnakeSegment *current = snake_head;

  while (current != NULL) {
    SDL_SetRenderDrawColor(renderer, current->color.r, current->color.g,
                           current->color.b, current->color.a);

    SDL_Rect segment_rect = {current->x, current->y, GRID_SIZE, GRID_SIZE};

    SDL_RenderFillRect(renderer, &segment_rect);

    current = current->next;
  }
}

// Render the food
void renderFood(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, food.color.r, food.color.g, food.color.b,
                         food.color.a);

  SDL_Rect food_rect = {food.x, food.y, GRID_SIZE, GRID_SIZE};

  SDL_RenderFillRect(renderer, &food_rect);
}

// Render the score
void renderScore(SDL_Renderer *renderer, TTF_Font *font) {
  char score_text[32];
  sprintf(score_text, "Score: %d", score);

  SDL_Color white = {255, 255, 255, 255};
  SDL_Surface *text_surface = TTF_RenderText_Solid(font, score_text, white);
  if (!text_surface) {
    printf("Failed to render text: %s\n", TTF_GetError());
    return;
  }

  SDL_Texture *text_texture =
      SDL_CreateTextureFromSurface(renderer, text_surface);
  if (!text_texture) {
    printf("Failed to create texture: %s\n", SDL_GetError());
    SDL_FreeSurface(text_surface);
    return;
  }

  SDL_Rect text_rect = {10, 10, text_surface->w, text_surface->h};
  SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);

  SDL_FreeSurface(text_surface);
  SDL_DestroyTexture(text_texture);
}

// Render game over message
void renderGameOver(SDL_Renderer *renderer, TTF_Font *font) {
  SDL_Color red = {255, 0, 0, 255};
  SDL_Surface *text_surface = TTF_RenderText_Solid(font, "Game Over", red);
  if (!text_surface) {
    printf("Failed to render text: %s\n", TTF_GetError());
    return;
  }

  SDL_Texture *text_texture =
      SDL_CreateTextureFromSurface(renderer, text_surface);
  if (!text_texture) {
    printf("Failed to create texture: %s\n", SDL_GetError());
    SDL_FreeSurface(text_surface);
    return;
  }

  SDL_Rect text_rect = {SCREEN_WIDTH / 2 - text_surface->w / 2,
                        SCREEN_HEIGHT / 2 - text_surface->h / 2,
                        text_surface->w, text_surface->h};

  SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);

  SDL_FreeSurface(text_surface);
  SDL_DestroyTexture(text_texture);
}

int main() {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }

  // Initialize SDL_ttf
  if (TTF_Init() == -1) {
    printf("TTF_Init: %s\n", TTF_GetError());
    return -1;
  }

  // Load font
  TTF_Font *font = TTF_OpenFont("Arial.ttf", 24);
  if (!font) {
    printf("Failed to load font: %s\n", TTF_GetError());
    SDL_Quit();
    return -1;
  }

  // Create window
  SDL_Window *window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                                        SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

  if (window == NULL) {
    SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }

  // Create renderer
  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (renderer == NULL) {
    SDL_Log("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }

  // Initialize game
  initializeGame();

  // Game loop variables
  bool quit = false;
  SDL_Event event;
  Uint32 last_move_time = 0;
  int move_delay = 1000 / GAME_SPEED;

  // Game loop
  while (!quit) {
    // Handle events
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
      } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_UP:
          if (current_direction != DOWN) {
            next_direction = UP;
          }
          break;
        case SDLK_RIGHT:
          if (current_direction != LEFT) {
            next_direction = RIGHT;
          }
          break;
        case SDLK_DOWN:
          if (current_direction != UP) {
            next_direction = DOWN;
          }
          break;
        case SDLK_LEFT:
          if (current_direction != RIGHT) {
            next_direction = LEFT;
          }
          break;
        case SDLK_r:
          // Reset game on 'R' key press
          if (game_over) {
            initializeGame();
          }
          break;
        case SDLK_ESCAPE:
          quit = true;
          break;
        }
      }
    }

    // Get current time
    Uint32 current_time = SDL_GetTicks();

    // Update game state at regular intervals
    if (!game_over && current_time - last_move_time >= move_delay) {
      // Update direction
      current_direction = next_direction;

      // Update snake position
      updatePositions();

      // Check for collisions
      // if (checkCollisionWithWall() || checkCollisionWithSelf()) {
      //   game_over = true;
      // }

      if (checkCollisionWithWall()) {
        game_over = true;
      }

      // Check if snake ate food
      if (checkCollisionWithFood()) {
        score++;
        should_grow = true;
        generateFood();
      }

      last_move_time = current_time;
    }

    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render game elements
    renderSnake(renderer);
    renderFood(renderer);
    renderScore(renderer, font);

    // Render game over message if game is over
    if (game_over) {
      renderGameOver(renderer, font);
    }

    // Update screen
    SDL_RenderPresent(renderer);

    // Cap frame rate
    SDL_Delay(1000 / 60); // 60 FPS
  }

  // Cleanup
  cleanupGame();
  TTF_CloseFont(font);
  TTF_Quit();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
