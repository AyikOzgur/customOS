#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>


#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64
#define SEND_PORT 7033
#define RECV_PORT 7034

int main()
{
    char input[MAX_INPUT_SIZE];

    // Define the library path you want to set
    const char *libraryPath = "/lib";

    // Create a string with the new LD_LIBRARY_PATH value
    char ldLibraryPath[1024];
    snprintf(ldLibraryPath, sizeof(ldLibraryPath), "LD_LIBRARY_PATH=%s", libraryPath);

    // Set the environment variable using putenv
    if (putenv(ldLibraryPath) != 0)
    {
        perror("putenv");
        return 1;
    }

    // Pid of child.
    pid_t pid;


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
                    {
                        perror("chdir");
                    }
                }
                else
                {
                    // Handle 'cd' with no arguments as 'cd ~'
                    if (chdir(getenv("HOME")) == -1)
                    {
                        perror("chdir");
                    }
                }
            }
            else
            {
                pid = fork();

                if (pid == -1)
                {
                    perror("fork");
                    return 1;
                }
                else if (pid == 0)
                {
                    // Check if the executable is in the current directory
                    char current_dir_path[1024];
                    snprintf(current_dir_path, sizeof(current_dir_path), "./%s", args[0]);
                    if (access(current_dir_path, X_OK) == 0)
                    {
                        execvp(current_dir_path, args);
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
