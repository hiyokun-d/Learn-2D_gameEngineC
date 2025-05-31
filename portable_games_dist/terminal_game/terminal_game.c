#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Cross-platform compatibility
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

// Constants
#define WIDTH 30
#define HEIGHT 15
#define QUEUE_SIZE 500

// Game entities
#define WALL '#'
#define PLAYER 'P'
#define EXIT 'E'
#define PATH ' '
#define VISITED '.'
#define TREASURE '*'

// Key codes
#define KEY_UP 'w'
#define KEY_DOWN 's'
#define KEY_LEFT 'a'
#define KEY_RIGHT 'd'
#define KEY_QUIT 'q'
#define KEY_RESET 'r'

// Directions for maze generation
#define DIR_UP 0
#define DIR_RIGHT 1
#define DIR_DOWN 2
#define DIR_LEFT 3

// Colors (ANSI escape codes)
#define COLOR_RESET "\033[0m"
#define COLOR_PLAYER "\033[1;32m"  // Bright Green
#define COLOR_WALL "\033[1;36m"    // Bright Cyan
#define COLOR_EXIT "\033[1;31m"    // Bright Red
#define COLOR_PATH "\033[0m"       // Default
#define COLOR_VISITED "\033[1;34m" // Bright Blue
#define COLOR_TREASURE "\033[1;33m" // Bright Yellow

// Position structure
typedef struct {
    int x, y;
} Position;

// Queue structure for maze generation and pathfinding
typedef struct {
    Position items[QUEUE_SIZE];
    int front, rear;
} Queue;

// Game state
char maze[HEIGHT][WIDTH];
Position player;
Position exit_pos;
int treasures_collected = 0;
int total_treasures = 0;
int moves = 0;
int game_time = 0;
bool game_won = false;

// Function prototypes
void initTerminal();
void resetTerminal();
char getch();
void clearScreen();
void initMaze();
void generateMaze();
void placeTreasures(int count);
void renderMaze();
void handleInput(char input);
bool movePlayer(int dx, int dy);
void resetGame();
bool isValidPosition(int x, int y);
void queueInit(Queue *q);
bool queueIsEmpty(Queue *q);
void queueEnqueue(Queue *q, Position pos);
Position queueDequeue(Queue *q);
void carvePath(int x, int y);

// Non-Windows terminal setup
#ifndef _WIN32
struct termios orig_termios;

void initTerminal() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void resetTerminal() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

char getch() {
    char c;
    // Set terminal to non-blocking mode
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    
    read(STDIN_FILENO, &c, 1);
    
    // Reset to blocking mode
    fcntl(STDIN_FILENO, F_SETFL, flags);
    return c;
}

#else
// Windows terminal setup
void initTerminal() {
    // No special setup needed for Windows
}

void resetTerminal() {
    // No special reset needed for Windows
}

// getch is already defined in conio.h for Windows
#endif

// Clear the screen
void clearScreen() {
    system(CLEAR_SCREEN);
}

// Queue operations
void queueInit(Queue *q) {
    q->front = q->rear = -1;
}

bool queueIsEmpty(Queue *q) {
    return q->front == -1;
}

void queueEnqueue(Queue *q, Position pos) {
    if ((q->rear + 1) % QUEUE_SIZE == q->front) {
        // Queue is full
        return;
    }
    
    if (q->front == -1) {
        q->front = 0;
    }
    
    q->rear = (q->rear + 1) % QUEUE_SIZE;
    q->items[q->rear] = pos;
}

Position queueDequeue(Queue *q) {
    Position pos = {-1, -1};
    
    if (queueIsEmpty(q)) {
        return pos;
    }
    
    pos = q->items[q->front];
    
    if (q->front == q->rear) {
        // Last element
        q->front = q->rear = -1;
    } else {
        q->front = (q->front + 1) % QUEUE_SIZE;
    }
    
    return pos;
}

// Initialize the maze with walls
void initMaze() {
    // Fill maze with walls
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            maze[y][x] = WALL;
        }
    }
}

// Check if a position is valid (within bounds)
bool isValidPosition(int x, int y) {
    return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
}

// Recursive backtracking maze generation
void carvePath(int x, int y) {
    // Mark current cell as path
    maze[y][x] = PATH;
    
    // Directions: up, right, down, left
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};
    
    // Shuffle directions
    int dirs[4] = {0, 1, 2, 3};
    for (int i = 0; i < 4; i++) {
        int j = rand() % 4;
        int temp = dirs[i];
        dirs[i] = dirs[j];
        dirs[j] = temp;
    }
    
    // Try each direction
    for (int i = 0; i < 4; i++) {
        int dir = dirs[i];
        int nx = x + dx[dir] * 2; // Move two cells in this direction
        int ny = y + dy[dir] * 2;
        
        if (isValidPosition(nx, ny) && maze[ny][nx] == WALL) {
            // Carve through the wall between current cell and next cell
            maze[y + dy[dir]][x + dx[dir]] = PATH;
            // Recursively carve from the next cell
            carvePath(nx, ny);
        }
    }
}

// Generate a random maze
void generateMaze() {
    initMaze();
    
    // Start carving from a random position (must be odd coordinates)
    int start_x = 1 + 2 * (rand() % ((WIDTH - 1) / 2));
    int start_y = 1 + 2 * (rand() % ((HEIGHT - 1) / 2));
    
    // Ensure start coordinates are within bounds
    if (start_x >= WIDTH) start_x = WIDTH - 2;
    if (start_y >= HEIGHT) start_y = HEIGHT - 2;
    
    // Start carving
    carvePath(start_x, start_y);
    
    // Set player position at a random path position
    do {
        player.x = rand() % (WIDTH - 2) + 1;
        player.y = rand() % (HEIGHT - 2) + 1;
    } while (maze[player.y][player.x] != PATH);
    
    // Set exit position at a random path position far from player
    do {
        exit_pos.x = rand() % (WIDTH - 2) + 1;
        exit_pos.y = rand() % (HEIGHT - 2) + 1;
        // Ensure exit is far enough from player
        int dx = abs(exit_pos.x - player.x);
        int dy = abs(exit_pos.y - player.y);
    } while (maze[exit_pos.y][exit_pos.x] != PATH || 
             (abs(exit_pos.x - player.x) + abs(exit_pos.y - player.y)) < (WIDTH + HEIGHT) / 3);
    
    // Place the exit
    maze[exit_pos.y][exit_pos.x] = EXIT;
}

// Place treasures in the maze
void placeTreasures(int count) {
    total_treasures = count;
    treasures_collected = 0;
    
    for (int i = 0; i < count; i++) {
        int x, y;
        do {
            x = rand() % (WIDTH - 2) + 1;
            y = rand() % (HEIGHT - 2) + 1;
        } while (maze[y][x] != PATH || 
                 (x == player.x && y == player.y) || 
                 (x == exit_pos.x && y == exit_pos.y));
        
        maze[y][x] = TREASURE;
    }
}

// Render the maze
void renderMaze() {
    clearScreen();
    
    // Display game info
    printf("Terminal Maze Explorer | Moves: %d | Treasures: %d/%d\n", 
           moves, treasures_collected, total_treasures);
    printf("Controls: WASD = Move, Q = Quit, R = Reset\n\n");
    
    // Draw the maze
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            // Check if this is the player's position
            if (x == player.x && y == player.y) {
                printf("%sP%s", COLOR_PLAYER, COLOR_RESET);
            } else {
                // Otherwise draw the maze element with appropriate color
                switch (maze[y][x]) {
                    case WALL:
                        printf("%s%c%s", COLOR_WALL, WALL, COLOR_RESET);
                        break;
                    case EXIT:
                        printf("%s%c%s", COLOR_EXIT, EXIT, COLOR_RESET);
                        break;
                    case VISITED:
                        printf("%s%c%s", COLOR_VISITED, VISITED, COLOR_RESET);
                        break;
                    case TREASURE:
                        printf("%s%c%s", COLOR_TREASURE, TREASURE, COLOR_RESET);
                        break;
                    default:
                        printf("%s%c%s", COLOR_PATH, maze[y][x], COLOR_RESET);
                }
            }
        }
        printf("\n");
    }
    
    // Display game status
    if (game_won) {
        printf("\nCongratulations! You found the exit!\n");
        printf("Final Score: %d (Lower is better)\n", moves);
        printf("Press 'R' to play again or 'Q' to quit\n");
    }
}

// Handle player input
void handleInput(char input) {
    if (game_won) {
        if (input == KEY_RESET) {
            resetGame();
        }
        return;
    }
    
    switch (input) {
        case KEY_UP:
            movePlayer(0, -1);
            break;
        case KEY_DOWN:
            movePlayer(0, 1);
            break;
        case KEY_LEFT:
            movePlayer(-1, 0);
            break;
        case KEY_RIGHT:
            movePlayer(1, 0);
            break;
        case KEY_RESET:
            resetGame();
            break;
    }
}

// Move the player
bool movePlayer(int dx, int dy) {
    int new_x = player.x + dx;
    int new_y = player.y + dy;
    
    // Check if the new position is valid
    if (!isValidPosition(new_x, new_y) || maze[new_y][new_x] == WALL) {
        return false;
    }
    
    moves++;
    
    // Check if the player reached the exit
    if (maze[new_y][new_x] == EXIT) {
        game_won = true;
    }
    
    // Check if the player found a treasure
    if (maze[new_y][new_x] == TREASURE) {
        treasures_collected++;
    }
    
    // Mark the current position as visited
    if (maze[player.y][player.x] != EXIT) {
        maze[player.y][player.x] = VISITED;
    }
    
    // Update player position
    player.x = new_x;
    player.y = new_y;
    
    return true;
}

// Reset the game
void resetGame() {
    generateMaze();
    placeTreasures(5 + rand() % 6); // Place 5-10 treasures
    moves = 0;
    game_won = false;
}

// Main function
int main() {
    // Initialize random seed
    srand((unsigned int)time(NULL));
    
    // Terminal setup
    initTerminal();
    
    // Initialize the game
    resetGame();
    
    // Game loop
    bool running = true;
    char input;
    
    while (running) {
        // Render the maze
        renderMaze();
        
        // Get input
        #ifdef _WIN32
        if (_kbhit()) {
            input = getch();
            
            // Handle arrow keys in Windows (arrow keys return two bytes)
            if (input == 0 || input == 224) {
                input = getch();
                switch (input) {
                    case 72: input = KEY_UP; break;    // Up arrow
                    case 80: input = KEY_DOWN; break;  // Down arrow
                    case 75: input = KEY_LEFT; break;  // Left arrow
                    case 77: input = KEY_RIGHT; break; // Right arrow
                }
            }
            
            // Handle quit
            if (input == KEY_QUIT) {
                running = false;
            } else {
                handleInput(input);
            }
        }
        #else
        // Unix/Mac input handling
        input = getch();
        
        // Handle arrow keys in Unix (arrow keys return escape sequence)
        if (input == 27) {
            // Skip the '['
            getch();
            input = getch();
            switch (input) {
                case 'A': input = KEY_UP; break;    // Up arrow
                case 'B': input = KEY_DOWN; break;  // Down arrow
                case 'D': input = KEY_LEFT; break;  // Left arrow
                case 'C': input = KEY_RIGHT; break; // Right arrow
            }
        }
        
        // Handle quit
        if (input == KEY_QUIT) {
            running = false;
        } else {
            handleInput(input);
        }
        #endif
        
        // Delay for a bit
        #ifdef _WIN32
        Sleep(100);
        #else
        usleep(100000);
        #endif
    }
    
    // Reset terminal settings
    resetTerminal();
    
    return 0;
}

