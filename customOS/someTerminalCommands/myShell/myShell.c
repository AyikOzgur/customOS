#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

int main()
{
    // Temporary solution for LD_LIBRARY_PATH. @todo: Find problem of setenv for LD_LIBRARY_PATH.
    const char *libraryPath = "/lib";
    char ldLibraryPath[1024];
    snprintf(ldLibraryPath, sizeof(ldLibraryPath), "LD_LIBRARY_PATH=%s", libraryPath);
    if (putenv(ldLibraryPath) != 0)
        perror("LD_LIBRARY_PATH could not set.");

    setenv("HOME", "/", 1);
    //setenv("LD_LIBRARY_PATH", "/lib", 1); // Kernel panics.

    pid_t pid;
    char input[MAX_INPUT_SIZE];

    while (1)
    {
        char pwd[1024];
        getcwd(pwd, sizeof(pwd));
        printf("myShell @ %s > ", pwd);
        fflush(stdout);

        // Get command and arguments.
        if (fgets(input, sizeof(input), stdin) == NULL) 
        {
            printf("Error reading input.\n");
            continue;
        }

        // Remove the trailing newline character
        input[strcspn(input, "\n")] = '\0';

        char *args[MAX_ARGS];
        int argc = 0;

        char *token = strtok(input, " ");

        while (token != NULL && argc < MAX_ARGS)
        {
            args[argc++] = token;
            token = strtok(NULL, " ");
        }

        args[argc] = NULL; // Null-terminate the argument list

        if (argc > 0)
        {
            if (strcmp(args[0], "cd") == 0)
            {
                if (args[1] != NULL)
                {
                    if (chdir(args[1]) == -1)
                        printf("Can't change directory to %s\n", args[1]);
                }
                else
                {
                    // Handle 'cd' with no arguments as 'cd ~'
                    if (chdir(getenv("HOME")) == -1)
                        perror("Can't change directory to home");
                }
            }
            else
            {
                pid = fork();

                if (pid == -1)
                {
                    perror("fork failed.");
                    continue;
                }
                else if (pid == 0)
                {
                    // Check if the executable is in the current directory
                    char currentDirPath[1024];
                    snprintf(currentDirPath, sizeof(currentDirPath), "./%s", args[0]);
                    if (access(currentDirPath, X_OK) == 0)
                    {
                        execvp(currentDirPath, args);
                    }
                    else
                    {
                        // Check if the executable is in /bin
                        char bin_path[1024];
                        snprintf(bin_path, sizeof(bin_path), "/bin/%s", args[0]);
                        if (access(bin_path, X_OK) == 0)
                        {
                            execvp(bin_path, args);
                        }
                        else
                        {
                            printf("Invalid command.\n");
                        }
                    }

                    exit(0); // Terminate the child process
                }
                else
                {
                    wait(pid);
                }
            }
        }
    }
    
    return 0;
}