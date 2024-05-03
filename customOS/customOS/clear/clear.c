#include <stdio.h>

int main()
{
    printf("\033[2J\033[1;1H"); // ANSI escape sequence to clear screen and move cursor to top left
    fflush(stdout);             // Flush stdout to ensure the output is sent immediately
    printf("\033[2J\033[1;1H"); // ANSI escape sequence to clear screen and move cursor to top left
    fflush(stdout);             // Flush stdout to ensure the output is sent immediately
    return 0;
}