# 2D Game Engine in C - Implementation Analysis

## Table of Contents
1. [Project Overview](#project-overview)
2. [Breakout Game Analysis](#breakout-game-analysis)
   - [Data Structures](#data-structures)
   - [Linked List Implementation](#linked-list-implementation)
   - [Game Mechanics](#game-mechanics)
   - [Memory Management](#memory-management)
3. [Snake Game Analysis](#snake-game-analysis)
   - [Doubly Linked List Implementation](#doubly-linked-list-implementation)
   - [Key Algorithms](#snake-key-algorithms)
4. [Terminal Maze Game Analysis](#terminal-maze-game-analysis)
   - [2D Array and Queue Implementation](#2d-array-and-queue-implementation)
   - [Cross-Platform Compatibility](#cross-platform-compatibility)
5. [Compilation and Running](#compilation-and-running)
6. [Conclusion](#conclusion)

## Project Overview

This project consists of three different games implementing various data structures in C:

1. **Breakout Game** (main.c): A classic brick-breaking game implementing a **singly linked list** for block management.
2. **Snake Game** (snake_game.c): A snake movement game implementing a **doubly linked list** for the snake's body.
3. **Terminal Maze** (terminal_game.c): A text-based maze navigation game implementing **2D arrays** for the maze and a **queue** for maze generation.

All games demonstrate fundamental computer science concepts, data structures, and game development techniques using the C programming language.

## Breakout Game Analysis

### Data Structures

The Breakout game implements three main data structures:

#### 1. Rectangle Structure
```c
typedef struct {
  int x, y, w, h;
  SDL_Color color; // property of RGBA which stands for (RED, GREEN, BLUE, ALPHA)
} Rectangle;
```
This structure represents rectangular objects like the player paddle and breakable blocks.

#### 2. Arc Structure
```c
struct Arc {
  int x, y;
  int r;
  double startAngle;
  double endAngle;
  SDL_Color color;
};
```
This structure represents the ball as a circular object with radius and position.

#### 3. Block Node (Singly Linked List)
```c
typedef struct BlockNode {
  Rectangle block;
  struct BlockNode *next;
} BlockNode;
```
This is the core data structure of the game: a singly linked list node that contains a Rectangle (the block itself) and a pointer to the next block.

#### Visual Representation of the Linked List

```
       ┌─────────┐    ┌─────────┐    ┌─────────┐
head-->│ Block 1 │--->│ Block 2 │--->│ Block 3 │---> ... ---> NULL
       └─────────┘    └─────────┘    └─────────┘
```

Each block node contains:
```
┌─────────────────────────┐
│ BlockNode               │
│ ┌───────────────────┐   │
│ │ Rectangle block   │   │
│ │ ┌─────────────┐   │   │
│ │ │ x, y, w, h   │   │   │
│ │ │ color        │   │   │
│ │ └─────────────┘   │   │
│ └───────────────────┘   │
│                         │
│ BlockNode *next ────────┼──> to next node
└─────────────────────────┘
```

### Linked List Implementation

The game creates and manages blocks using a linked list implementation. This provides several advantages:
- Dynamic addition and removal of blocks
- Efficient memory usage
- O(1) removal when a block is hit by the ball

#### Creating the Linked List

```c
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
```

This function:
1. Creates a specified number of blocks
2. Arranges them in rows and columns
3. Assigns random colors to each block
4. Links them together in a singly linked list
5. Returns the head of the list

#### Drawing the Blocks

```c
void drawBlocks(SDL_Renderer *renderer, BlockNode *head) {
  BlockNode *current = head;
  while (current != NULL) {
    drawRectangle(renderer, current->block);
    current = current->next;
  }
}
```

This function iterates through the linked list, drawing each block on the screen.

#### Breaking Blocks

```c
void breakBlock(struct Arc ball, BlockNode **head) {
  BlockNode *current = *head;
  BlockNode *prev = NULL;

  while (current != NULL) {
    Rectangle blk = current->block;
    // AABB collision detection
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
```

This function:
1. Checks for collision between the ball and each block
2. When a collision is detected, removes the block from the linked list
3. Updates the score and other game variables
4. Properly frees the memory of the removed block

### Game Mechanics

#### Collision Detection

The game uses Axis-Aligned Bounding Box (AABB) collision detection:

```c
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
```

#### Ball Movement

The ball movement is handled by updating its position based on velocity:

```c
ball_x += ball_vx;
ball_y += ball_vy;
```

When the ball collides with the paddle, walls, or blocks, its velocity is adjusted to create bouncing behavior:

```c
if (checkCollision(ball, playerBlock)) {
  ball_vy = -ballSpeed;
}

if (ball_x < 0) {
  ball_vx = ballSpeed;
}

if (ball_x > SCREEN_WIDTH - ball.r) {
  ball_vx = -ballSpeed;
}
```

### Memory Management

The game handles memory allocation and deallocation properly to prevent memory leaks:

1. **Allocation**: Memory is allocated for each block when creating the linked list.
2. **Deallocation during gameplay**: When a block is hit, its memory is freed.
3. **Cleanup at game end**: All remaining blocks are freed when the game ends.

```c
// Cleanup at end of game
BlockNode *current = blockList;
while (current != NULL) {
  BlockNode *next = current->next;
  free(current);
  current = next;
}
```

## Snake Game Analysis

The Snake game implements a **doubly linked list** to represent the snake's body segments.

### Doubly Linked List Implementation

#### Snake Segment Structure
```c
typedef struct SnakeSegment {
  int x, y;                  // Position of segment
  SDL_Color color;           // Color of segment
  struct SnakeSegment *prev; // Pointer to previous segment
  struct SnakeSegment *next; // Pointer to next segment
} SnakeSegment;
```

#### Visual Representation

```
       ┌─────────┐    ┌─────────┐    ┌─────────┐
NULL<--│ Head    │<-->│ Body    │<-->│ Tail    │---> NULL
       └─────────┘    └─────────┘    └─────────┘
```

#### Key Operations

1. **Insert Head**: Adds a new segment at the head of the snake
```c
void insertHead(int x, int y) {
  // Create new head segment
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
```

2. **Delete Tail**: Removes the last segment of the snake
```c
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
```

### Snake Key Algorithms

1. **Movement**: The snake moves by adding a new head segment in the direction of movement and removing the tail segment (unless growing).
2. **Growth**: When the snake eats food, it grows by adding a new head segment without removing the tail.
3. **Collision Detection**: Checks for collisions with walls, food, and the snake's own body.

## Terminal Maze Game Analysis

The Terminal Maze game implements two main data structures:

### 2D Array and Queue Implementation

#### 2D Array for Maze Representation
```c
// Game state
char maze[HEIGHT][WIDTH];
```

The maze is represented as a 2D array of characters, where different characters represent different elements:
- `#` - Wall
- `P` - Player
- `E` - Exit
- ` ` - Path
- `.` - Visited path
- `*` - Treasure

#### Queue for Maze Generation
```c
// Queue structure for maze generation and pathfinding
typedef struct {
    Position items[QUEUE_SIZE];
    int front, rear;
} Queue;
```

The queue is used for maze generation using a modified breadth-first search approach.

### Cross-Platform Compatibility

The terminal game is designed to work on both Windows and Unix-based systems by using conditional compilation:

```c
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#define CLEAR_SCREEN "cls"
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#define CLEAR_SCREEN "clear"
#endif
```

## Compilation and Running

### Breakout and Snake Games

These games require SDL2 and SDL2_ttf libraries:

**macOS:**
```bash
# Install dependencies
./install_sdl_mac.sh

# Compile and run
gcc main.c -o breakout $(sdl2-config --cflags --libs) -lSDL2_ttf && ./breakout
gcc snake_game.c -o snake_game $(sdl2-config --cflags --libs) -lSDL2_ttf && ./snake_game
```

**Windows:**
```bash
# Setup portable distribution
./make_portable_distribution.sh

# Use the generated files in portable_games_dist folder
```

### Terminal Maze Game

The terminal game doesn't require external libraries:

```bash
# macOS/Linux
gcc terminal_game.c -o terminal_maze && ./terminal_maze

# Windows
gcc terminal_game.c -o terminal_maze.exe
```

## Conclusion

These three games demonstrate different data structures and their applications in game development:

1. **Breakout Game**: Singly linked list for dynamic block management
2. **Snake Game**: Doubly linked list for efficient snake body segment management
3. **Terminal Maze Game**: 2D array and queue for maze representation and generation

Key programming concepts demonstrated:
- Dynamic memory allocation and management
- Pointer manipulation
- Data structure traversal and modification
- Game logic implementation
- Cross-platform development
- Graphics rendering (SDL2)
- User input handling

These implementations showcase how choosing the right data structure can simplify development and improve performance in game programming.

