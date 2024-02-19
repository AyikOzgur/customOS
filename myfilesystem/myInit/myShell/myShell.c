#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

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

    // Socket file descripter.
    int sockfd;
    // My address to bind for recieving data.
    struct sockaddr_in myAddr;
    // Pipes for directing child process in and out
    int parent_to_child[2], child_to_parent[2];
    // Pid of child.
    pid_t pid;

    // Create socket.
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Sender socket creation failed");
    }

    // Set up my address for receiving data
    memset(&myAddr, 0, sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(RECV_PORT);
    myAddr.sin_addr.s_addr = inet_addr("192.168.0.2");

    // Bind the socket to the port for receiving data
    if (bind(sockfd, (struct sockaddr *)&myAddr, sizeof(myAddr)) < 0)
    {
        perror("Bind failed");
    }

    // Prepare destionation address of remote connection.
    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(SEND_PORT);
    destAddr.sin_addr.s_addr = inet_addr("192.168.0.3");

    // Pipe creation
    if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        char pwd[1024];
        getcwd(pwd, sizeof(pwd));
        printf("myShell @ %s > ", pwd);
        fflush(stdout);

        // fd set for entering command from stdin or remote
        fd_set readfdsFirst;
        FD_ZERO(&readfdsFirst);
        FD_SET(sockfd, &readfdsFirst);       // Monitor socket for incoming data
        FD_SET(STDIN_FILENO, &readfdsFirst); // Monitor pipe for stdin

        // Select the highest file descriptor for the select call
        int maxfdFirst = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

        // Wait for an activity on either stdin or socket
        if (select(maxfdFirst + 1, &readfdsFirst, NULL, NULL, NULL) < 0)
        {
            perror("select");
            continue;
        }

        // If there's input from the network
        if (FD_ISSET(sockfd, &readfdsFirst))
        {
            struct sockaddr_in senderAddr;
            socklen_t senderAddrLen = sizeof(senderAddr);
            char Buffer[MAX_INPUT_SIZE];
            ssize_t messageLen = recvfrom(sockfd, Buffer, sizeof(Buffer) - 1, 0, (struct sockaddr *)&senderAddr, &senderAddrLen);
            if (messageLen > 0)
            {
                memcpy(input, Buffer, messageLen);
            }
        }

        // If there's data from the child's stdout
        else if (FD_ISSET(STDIN_FILENO, &readfdsFirst))
        {
            char Buffer[MAX_INPUT_SIZE];
            ssize_t bytesRead = read(STDIN_FILENO, Buffer, sizeof(Buffer) - 1);
            memcpy(input, Buffer, bytesRead);
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

        // Reinitialize pipes for each command
        if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1)
        {
            perror("pipe");
            // exit(EXIT_FAILURE);
        }

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
                    close(parent_to_child[1]); // Close unused write end
                    close(child_to_parent[0]); // Close unused read end

                    dup2(parent_to_child[0], STDIN_FILENO);  // Redirect stdin
                    dup2(child_to_parent[1], STDOUT_FILENO); // Redirect stdout

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
                    // Parent process
                    close(parent_to_child[0]); // Close unused read end
                    close(child_to_parent[1]); // Close unused write end
                    char buffer[1024];

                    ssize_t nread, nrecv;

                    while (1)
                    {
                        // Inside your while loop
                        int status;
                        pid_t result = waitpid(pid, &status, WNOHANG);
                        if (result > 0)
                        {
                            // Child has exited, break out of the loop
                            break;
                        }
                        else if (result == -1)
                        {
                            // An error occurred with waitpid
                            perror("waitpid");
                            break; // Optionally break here as well, as an error might indicate a problem
                        }

                        fd_set readfdsSecond;
                        FD_ZERO(&readfdsSecond);
                        FD_SET(sockfd, &readfdsSecond);             // Monitor socket for incoming data
                        FD_SET(child_to_parent[0], &readfdsSecond); // Monitor pipe for data from child's stdout

                        // Select the highest file descriptor for the select call
                        int maxfdSecond = sockfd > child_to_parent[0] ? sockfd : child_to_parent[0];

                        // Wait for an activity on either stdin or socket
                        if (select(maxfdSecond + 1, &readfdsSecond, NULL, NULL, NULL) < 0)
                        {
                            perror("select");
                            continue;
                        }

                        // If there's input from the network
                        if (FD_ISSET(sockfd, &readfdsSecond))
                        {
                            struct sockaddr_in senderAddr;
                            socklen_t senderAddrLen = sizeof(senderAddr);
                            char buffer[MAX_INPUT_SIZE];
                            ssize_t messageLen = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&senderAddr, &senderAddrLen);
                            if (messageLen > 0)
                            {
                                buffer[messageLen] = '\0'; // in case.
                                // Write the received message to the child's stdin via the pipe
                                ssize_t nwrite = write(parent_to_child[1], buffer, messageLen);
                                if (nwrite < 0)
                                {
                                    perror("write to child stdin failed");
                                }
                            }
                        }

                        // If there's data from the child's stdout
                        if (FD_ISSET(child_to_parent[0], &readfdsSecond))
                        {
                            char buffer[MAX_INPUT_SIZE];
                            ssize_t nread = read(child_to_parent[0], buffer, sizeof(buffer));
                            if (nread > 0)
                            {
                                // Send data read from child's stdout to remote
                                sendto(sockfd, buffer, nread, 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
                            }
                            else if (nread == 0)
                            {
                                // Child process closed its stdout, might have exited
                            }
                        }
                    }

                    // Close used ends of pipes (might need to reopen for next command)
                    close(parent_to_child[1]);
                    close(child_to_parent[0]);
                }
            }
        }
    }
    
    close(sockfd);

    return 0;
}
