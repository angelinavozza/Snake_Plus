#include <ncurses.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Snake and game variables
int snakeX[1840];           // X-coordinates of snake segments
int snakeY[1840];           // Y-coordinates of snake segments
int apples_x[100];          // X-coordinates of apples
int apples_y[100];          // Y-coordinates of apples
int obstacles_x[50];        // X-coordinates of obstacles
int obstacles_y[50];        // Y-coordinates of obstacles    

int snakeLength = 4;        // Initial length of the snake
int direction = 1;          // Initial direction (1 = right) 
int num_obstacles = 0;      // Number of obstacles on the screen
int level = 1;              // Initial game level
int level_completed = 0;    // Flag to check if the level is completed
int score = 0;              // Player's score

int timeout_val = 300;      // Time interval for the game loop (in ms)
int paused = 0;             // Flag to pause the game
int game_over = 0;          // Flag to end the game


// Function to process command-line arguments 
int arguments(int argc, char *argv[]) {
    // If the argument "-d" is passed, use a fixed seed for random number generation
    if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        srand(1234);
    } else {
        srand(time(NULL));              // Otherwise, use the current time as the seed
    }
    return 0;
}


// Initialize ncurses window and settings
void init_window() {
    initscr();                          // Initialize the screen
    raw();                              // Disable input buffering
    noecho();                           // Disable echoing of characters
    keypad(stdscr, TRUE);               // Enable keypad input (for arrow keys)
    curs_set(0);                        // Hide the cursor
    resize_term(24, 80);                // Set terminal size
    clear();                            // Clear the screen
    refresh();                          // Refresh the screen to apply changes

    start_color();                      // Initialize color functionality
    use_default_colors();               // Use default terminal colors
    init_pair(1, COLOR_YELLOW, -1);     // Snake color: Yellow on Black
    init_pair(2, COLOR_MAGENTA, -1);    // Headers color: Pink (Magenta) on Black
    init_pair(3, COLOR_RED, -1);        // Apples color: Red on Black
    init_pair(4, COLOR_BLUE, -1);       // Obstacles color: Blue on Black
}


// Create borders for the game area
void create_borders() {
    for (int i = 0; i < 80; i++) {
        mvaddch(1, i, '=');             // Top border
        mvaddch(23, i, '=');            // Bottom border
    }
    for (int j = 1; j < 24; j++) {
        mvaddch(j, 0, ']');             // Left border
        mvaddch(j, 79, '[');            // Right border
    }
}


// Generate apples at random positions
void generate_apples(int level) {
    char apple = '@';                                               // Apple character
    
    // Generate apples based on the level (more apples as level increases)
    for (int i = 0; i < level + 3; i++) {
        int x_coor_apple;
        int y_coor_apple;
        
        do {
            // Random position for apple (ensures it is not placed on a border)
            x_coor_apple = rand() % 78 + 1;
            y_coor_apple = rand() % 21 + 3;
        } while (mvinch(y_coor_apple, x_coor_apple) != ' ');        // Check if the position is empty
        
        apples_x[i] = x_coor_apple;
        apples_y[i] = y_coor_apple;

        attron(COLOR_PAIR(3));                                      // Set color to red for apples
        mvaddch(y_coor_apple, x_coor_apple, apple);                 // Draw the apple
        attroff(COLOR_PAIR(3));                                     // Reset color
    }
}


// Generate obstacles at random positions
void generate_obstacles(int level) {
    char obstacle = 'X';                                                    // Obstacle character

    do {
        // Random position for obstacle (ensures it is not placed on a border)
        obstacles_x[level - 1] = rand() % 78 + 1;
        obstacles_y[level - 1] = rand() % 21 + 3;
    } while (mvinch(obstacles_y[level - 1], obstacles_x[level - 1]) != ' ');

    attron(COLOR_PAIR(4));                                                  // Set color to blue for obstacles
    mvaddch(obstacles_y[level - 1], obstacles_x[level - 1], obstacle);      // Draw the obstacle
    attroff(COLOR_PAIR(4));                                                 // Reset color
    num_obstacles++;                                                        // Increment the obstacle count
}


// Initialize the snake at the starting position
void init_snake() {
    snakeY[0] = 12;
    snakeX[0] = 40;

    for (int i = 1; i < snakeLength; i++) {
        snakeY[i] = snakeY[0];             
        snakeX[i] = snakeX[i - 1] - 1;              // Snake body starts to the left
    }

    attron(COLOR_PAIR(1));                          // Set color to yellow for the snake
    mvaddch(snakeY[0], snakeX[0], 'O');             // Draw snake head
    for (int i = 1; i < snakeLength; i++) {
        mvaddch(snakeY[i], snakeX[i], '*');         // Draw snake body
    }
    attroff(COLOR_PAIR(1));                         // Reset color
}


// Move the snake based on the current direction
void move_snake() {
    int tail_x = snakeX[snakeLength - 1];           // Save the tail's position
    int tail_y = snakeY[snakeLength - 1];

    // Move the body: Shift all segments to the next position
    for (int i = snakeLength - 1; i > 0; i--) {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
    }

    // Move the head based on the current direction
    switch(direction) {
        case 0:  // Up
            snakeY[0] -= 1;
            break;
        case 1:  // Right
            snakeX[0] += 1;
            break;
        case 2:  // Down
            snakeY[0] += 1;
            break;
        case 3:  // Left
            snakeX[0] -= 1;
            break;
    }

    attron(COLOR_PAIR(1));                          // Set color to yellow for snake
    mvaddch(snakeY[0], snakeX[0], 'O');             // Draw head
    for (int i = 1; i < snakeLength; i++) {
        mvaddch(snakeY[i], snakeX[i], '*');         // Draw body
    }

    // Erase the tail
    mvaddch(tail_y, tail_x, ' ');

    attroff(COLOR_PAIR(1));                         // Reset color
    refresh();
}


// Handle user input (movement and exiting)
void handle_input() {
    int c = getch();                                // Block until a key is pressed
    switch(c) {
        case 'i':  // Up
            if (direction != 2) direction = 0;
            break;
        case 'k':  // Right
            if (direction != 3) direction = 1;
            break;
        case 'm':  // Down
            if (direction != 0) direction = 2;
            break;
        case 'j':  // Left
            if (direction != 1) direction = 3;
            break;
        case '0':  // Exit
            endwin();
            exit(0);
    }
}


// Grow the snake by one segment
void grow_snake() {
    snakeLength++;
    snakeX[snakeLength - 1] = snakeX[snakeLength - 2];
    snakeY[snakeLength - 1] = snakeY[snakeLength - 2];
}


// Check if the snake has eaten an apple
void check_for_apples() {
    for (int i = 0; i < level + 3; i++) {
        if (snakeX[0] == apples_x[i] && snakeY[0] == apples_y[i]) {
            score += level;                                             // Increase score based on the level
            beep();
            grow_snake();                                               // Grow the snake
            apples_x[i] = -1;
            apples_y[i] = -1;
            mvaddch(apples_y[i], apples_x[i], ' ');                     // Erase the eaten apple

            timeout_val = timeout_val * 0.98;                           // Increase game speed slightly
            timeout((int)(timeout_val));
        }
    }

    // Check if all apples are eaten
    int apples_left = 0;
    for (int i = 0; i < level + 3; i++) {
        if (apples_x[i] != -1 && apples_y[i] != -1) {
            apples_left = 1; 
            break;
        }
    }

    if (!apples_left) {
        level_completed = 1;                                            // Mark level as completed
    }
}


// Check if the snake collides with the walls, itself, or obstacles
void check_for_collisions() {
    // Wall collision
    if ((snakeX[0] < 1) || (snakeX[0] > 78 ) || (snakeY[0] < 2) || (snakeY[0] > 22)) {
        attron(COLOR_PAIR(3));
        mvprintw(1, 30, "Done, press key to exit");
        attroff(COLOR_PAIR(3));
        game_over = 1;
        return;
    }

    // Self-collision
    for (int i = 1; i < snakeLength; i++) {
        if ((snakeX[0] == snakeX[i]) && (snakeY[0] == snakeY[i])) {
            attron(COLOR_PAIR(3));
            mvprintw(1, 30, "Done, press key to exit");
            attroff(COLOR_PAIR(3));
            game_over = 1;
            return;
        }
    }

    // Obstacle collision
    for (int j = 0; j < num_obstacles; j++) {
        if ((snakeX[0] == obstacles_x[j]) && (snakeY[0] == obstacles_y[j])) {
            attron(COLOR_PAIR(3));
            mvprintw(1, 30, "Done, press key to exit");
            attroff(COLOR_PAIR(3));
            game_over = 1;
            return;
        }
    }
}


// Update the game labels (score, level, snake length)
void update_labels() {
    attron(COLOR_PAIR(2));                          // Set color to magenta for headers
    move(0, 0);
    clrtoeol();

    mvprintw(0, 0, "Level %d", level);
    mvprintw(0, 37, "Score %d", score);
    mvprintw(0, 69, "%d Segments", snakeLength);

    attroff(COLOR_PAIR(2));                         // Reset color
}


int main(int argc, char *argv[]) {
    arguments(argc, argv);                          // Handle command-line arguments
    init_window();                                  // Initialize ncurses window
    create_borders();                               // Create the game borders

    attron(COLOR_PAIR(3));                          // Set color for "Press key to start"
    mvprintw(1, 32, "Press key to start");
    attroff(COLOR_PAIR(3));                         // Reset color

    init_snake();                                   // Initialize the snake
    generate_apples(level);                         // Generate apples
    generate_obstacles(level);                      // Generate obstacles
    update_labels();                                // Display labels
    refresh();
    getch();                                        // Wait for user to start the game

    timeout(timeout_val);                           // Set the game loop timeout
    move(1, 0);                                     // Move the cursor to line 1, column 0
    clrtoeol();
    create_borders();                               // Create the borders again

    flushinp();                                     // Clear any leftover input

    while (!game_over) {
        update_labels();                            // Update game info labels
        move_snake();                               // Move the snake
        handle_input();                             // Handle user input
        check_for_apples();                         // Check for apple consumption
        check_for_collisions();                     // Check for collisions

        // If the level is completed, move to the next level
        if (level_completed) {
            ++level;
            beep();
            generate_apples(level);
            generate_obstacles(level);
            level_completed = 0;
        }

        refresh();
    }

    timeout(-1);                                    // Disable timeout after game ends

    beep();                                         // Play a sound
    beep();
    beep();

    flushinp();                                     // Clear any remaining input
    getch();                                        // Wait for a keypress to exit
    clear();                                        // Clear the screen
    endwin();                                       // End ncurses mode

    // Print final score and game stats
    printf("Your score was %d.\n", score);
    printf("Your snake had %d segments.\n", snakeLength);
    printf("You made it to level %d.\n", level);
    
    return 0;
}