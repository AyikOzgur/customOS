#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64
#define SEND_PORT 7033
#define RECV_PORT 7034


int main()
{
    struct timespec ts;
    ts.tv_sec = 0; // Seconds
    ts.tv_nsec = 5 * 1000000L; // 5 milliseconds in nanoseconds

    char command[MAX_INPUT_SIZE];

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

    char hello[] = "Hello from remote";
    sendto(sockfd, hello, sizeof(hello), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));

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
        char prompt[2048]; // Make sure this buffer is large enough to hold the resulting string

        struct sockaddr_in senderAddr;
        socklen_t senderAddrLen = sizeof(senderAddr);
        char commandBuffer[MAX_INPUT_SIZE];
        ssize_t messageLen = recvfrom(sockfd, commandBuffer, sizeof(commandBuffer) - 1, 0, (struct sockaddr *)&senderAddr, &senderAddrLen);
        if (messageLen > 0)
        {
            memcpy(command, commandBuffer, messageLen);
        }

        // Remove the trailing newline character
        command[strcspn(command, "\n")] = '\0';

        char *args[MAX_ARGS];
        int argc = 0;

        char *token = strtok(command, " ");

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
            else if(strcmp(args[0], "clear") == 0)
            {
                char clearCommand[] = "\033[2J\033[1;1H"; // ANSI escape sequence to clear screen and move cursor to top left
                sendto(sockfd, clearCommand, sizeof(clearCommand), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
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
                    // give so small sleep
                    nanosleep(&ts, NULL);

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
                    // give so small sleep
                    nanosleep(&ts, NULL);
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
                        fd_set readfds;
                        FD_ZERO(&readfds);
                        FD_SET(sockfd, &readfds);             // Monitor socket for incoming data
                        FD_SET(child_to_parent[0], &readfds); // Monitor pipe for data from child's stdout

                        // Select the highest file descriptor for the select call
                        int maxfd = sockfd > child_to_parent[0] ? sockfd : child_to_parent[0];

                        // Wait for an activity on either stdin or socket
                        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0)
                        {
                            perror("select failed");
                            continue;
                        }

                        // If there's data from the child's stdout
                        if (FD_ISSET(child_to_parent[0], &readfds))
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

                        // If there's input from the network
                        if (FD_ISSET(sockfd, &readfds))
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