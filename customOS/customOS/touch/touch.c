#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        printf("Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Open the file - create it if it doesn't exist
    int fd = open(argv[1], O_WRONLY | O_CREAT, 0666);
    if (fd == -1) 
    {
        printf("Error opening file '%s'\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    close(fd);

    // Update the access and modification times
    if (utime(argv[1], NULL) != 0) 
    {
        printf("Error updating time for file '%s'\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    return 0;
}