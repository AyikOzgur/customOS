#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/stat.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

void mntrootFilePart()
{
    // mount main root file system
    const char *source = "/dev/mmcblk0p2";
    const char *target = "/mnt";
    const char *filesystemtype = "ext4";
    unsigned long mountflags = 0;
    const void *data = NULL;
    sleep(1);

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
        chdir("/");
    }
}

void mntbootPart()
{
    // mount main root file system
    const char *source = "/dev/mmcblk0p1";
    const char *target = "/boot";
    const char *filesystemtype = "vfat";
    unsigned long mountflags = 0;
    const void *data = NULL;
    sleep(1);

    // Attempt to mount the device
    if (mount(source, target, filesystemtype, mountflags, data) != 0) 
    {
        perror("mount failed");
    }
    else
        printf("Mounted %s to %s\n", source, target);

}

int main() 
{
    mntrootFilePart();
    mntbootPart();
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

    char input[MAX_INPUT_SIZE];

    while (1) 
    {
        char pwd[1024];
        getcwd(pwd, sizeof(pwd));
        printf("myShell @ %s > ", pwd);
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
                pid_t pid = fork();

                if (pid == -1) 
                {
                    perror("fork");
                    return 1;
                } 
                else if (pid == 0) 
                {
                    // Child process
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

                    exit(1); // Terminate the child process
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
    }

    return 0;
}
