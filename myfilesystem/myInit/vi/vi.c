#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#define MAX_LINE_LENGTH 1000

// Function to clear the screen
void clear_screen() {
    printf("\033[2J\033[H");
}

// Function to move cursor to specified position
void move_cursor(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

// Function to enable raw mode for terminal input
void enable_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Function to disable raw mode for terminal input
void disable_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(int argc, char *argv[]) {
    FILE *file;
    char line[MAX_LINE_LENGTH];
    int ch;
    int cursor_x = 0;
    int cursor_y = 0;

    // Check if a file is provided as argument
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Open the file for reading and writing
    file = fopen(argv[1], "r+");
    if (file == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    // Enable raw mode for terminal input
    enable_raw_mode();

    // Clear the screen
    clear_screen();

    // Print initial contents of the file
    rewind(file);
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        printf("%s", line);
        cursor_y++;
    }

    // Move cursor to beginning of file
    move_cursor(0, 0);

    // Main loop for editing
    while ((ch = getchar()) != EOF) {
        // Handle arrow key presses (if possible)
        if (ch == '\033') { // ANSI escape sequence for arrow keys
            ch = getchar(); // Get the next character
            if (ch == '[') {
                ch = getchar(); // Get the third character
                if (ch == 'A') { // Up arrow
                    if (cursor_y > 0) {
                        cursor_y--;
                        move_cursor(cursor_x, cursor_y);
                    }
                } else if (ch == 'B') { // Down arrow
                    cursor_y++;
                    move_cursor(cursor_x, cursor_y);
                } else if (ch == 'C') { // Right arrow
                    cursor_x++;
                    move_cursor(cursor_x, cursor_y);
                } else if (ch == 'D') { // Left arrow
                    if (cursor_x > 0) {
                        cursor_x--;
                        move_cursor(cursor_x, cursor_y);
                    }
                }
            }
        } else {
            // Write the character to the file
            fputc(ch, file);
            fflush(file);

            // Print the character to the screen
            putchar(ch);

            // Move cursor position
            cursor_x++;

            // Handle line breaks
            if (ch == '\n') {
                cursor_x = 0;
                cursor_y++;
            }
        }
    }

    // Disable raw mode for terminal input
    disable_raw_mode();

    // Close the file
    fclose(file);

    return 0;
}
