#include <stdio.h>

#define MAX_LINE_LENGTH 1024

int main() {
    char line[MAX_LINE_LENGTH];

    printf("Enter a line (type 'q' to quit): ");
    fflush(stdout); // Make sure the prompt is displayed before user input

    while (fgets(line, MAX_LINE_LENGTH, stdin) != NULL) {
        // Check if the first character is 'q' and if it's the only character before the newline
        if (line[0] == 'q' && (line[1] == '\n' || line[1] == '\0')) {
            printf("\n'q' received, exiting program.\n");
            break; // Exit the loop, thus ending the program
        }

        // Print the read line
        printf("%s", line);

        // Print the prompt again for the next line of input
        printf("Enter a line (type 'q' to quit): ");
        fflush(stdout);
    }

    return 0; // Successful execution
}
