#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

int main() 
{
    const char *source = "/dev/mmcblk0p2";
    const char *target = "/mnt";
    const char *filesystemtype = "ext4";
    unsigned long mountflags = 0;
    const void *data = NULL;
    sleep(5);

    // Create the target mount directory with appropriate permissions
    if (mkdir(target, 0755) != 0) 
    {
        perror("mkdir failed");
    }

    // Attempt to mount the device
    if (mount(source, target, filesystemtype, mountflags, data) != 0) 
    {
        perror("mount failed");
    }
    else
        printf("Mounted %s to %s\n", source, target);

    // Change root
    if (chroot("/mnt") < 0)
    {
        perror("chroot");
    }
    else
    {
        printf("Root file system switched.\n");
    }

    char input[MAX_INPUT_SIZE];
    
    while (1) 
    {
        char pwd[1024];
        getcwd(pwd, sizeof(pwd));
        printf("mySell @ %s > ", pwd);
        fflush(stdout);
        
        if (fgets(input, sizeof(input), stdin) == NULL) 
        {
            break;
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

        if (argc > 0) {
            pid_t pid = fork();

            if (pid == -1) 
            {
                perror("fork");
                return 1;
            } 
            else if (pid == 0) 
            {
                // Child process
                if (strcmp(args[0], "cd") == 0)
                {
                    chdir(args[1]);
                }
                else if(strcmp(args[0], "ls") == 0) 
                {
                    args[0] = "/bin/my_ls";
                    execlp(args[0], args[0], args[1], args[2], args[3], NULL);
                    perror("execlp"); // This line will only be reached if execlp fails
                    return 1;
                }
                else if(strcmp(args[0], "mkdir") == 0) 
                {
                    args[0] = "/bin/my_mkdir";
                    execlp(args[0], args[0], args[1], args[2], args[3], NULL);
                    perror("execlp"); // This line will only be reached if execlp fails
                    return 1;
                }
                else
                {
                    printf("Invaild command.\n");
                }
            } 
            else 
            { 
                // Parent process
                int status;
                waitpid(pid, &status, 0);
                // The parent continues to run after the child process finishes.
            }
        }
    }
    
    return 0;
}
