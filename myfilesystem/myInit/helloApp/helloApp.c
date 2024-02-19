#include <stdio.h>

int main()
{
    char ch;

    printf("Enter characters (press 'q' to quit): ");
    fflush(stdout); // Make sure the prompt is displayed before user input

    while ((ch = getchar()) != EOF)
    {
        // Check if the read character is 'q'
        if (ch == 'q')
        {
            printf("\n'q' received, exiting program.\n");
            break; // Exit the loop, thus ending the program
        }

        // Print the read character
        printf("%c", ch);

        // If a newline is entered, print the prompt again for clarity
        if (ch == '\n')
        {
            printf("Enter characters (press 'q' to quit): ");
            fflush(stdout);
        }
    }

    return 0; // Successful execution
}