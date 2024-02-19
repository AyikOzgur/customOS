// udp_receiver.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#define RECV_PORT 7033
#define SEND_PORT 7034
#define BUFFER_SIZE 1024

void clear_udp_buffer(int sockfd) 
{
    char tmpBuffer[1024]; // Temporary buffer for discarding data
    ssize_t received;

    // Set socket to non-blocking mode
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    // Attempt to read until buffer is empty
    while ((received = recvfrom(sockfd, tmpBuffer, sizeof(tmpBuffer), 0, NULL, 0)) > 0) 
    {
        printf(" \r\n");
        //printf("    \r");
    }

    // Check if we stopped reading due to no more data or an error
    if (received < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("Error clearing buffer");
    }

    // Optionally, set socket back to blocking mode if needed
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) & ~O_NONBLOCK);
}


int main()
{
    int sockfd;
    struct sockaddr_in recvAddr, destAddr, senderAddr;
    char buffer[1024];
    socklen_t senderAddrLen = sizeof(senderAddr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Receiver socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up the receiver's address structure
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_addr.s_addr = inet_addr("192.168.0.3");
    recvAddr.sin_port = htons(RECV_PORT); // The port on which to listen for incoming data

    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(SEND_PORT);
    destAddr.sin_addr.s_addr = inet_addr("192.168.0.2");

    if (bind(sockfd, (struct sockaddr *)&recvAddr, sizeof(recvAddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    clear_udp_buffer(sockfd);

    printf("Listening on port %d\n", RECV_PORT);

    fd_set readfds;
    int max_fd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

    while (1)
    {
        clear_udp_buffer(sockfd);
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0)
        {
            perror("select failed");
            exit(EXIT_FAILURE);
        }

        // If data is received on the UDP socket
        if (FD_ISSET(sockfd, &readfds))
        {
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&senderAddr, &senderAddrLen);
            if (n < 0)
            {
                perror("recvfrom failed");
                continue;
            }
            buffer[n] = '\0'; // Null-terminate the received data
            printf("%s", buffer);
        }

        // If user has entered a message on stdin
        else if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            char Buffer[BUFFER_SIZE];
            ssize_t bytesRead = read(STDIN_FILENO, Buffer, sizeof(Buffer) - 1);
            sendto(sockfd, Buffer, strlen(Buffer), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
        }
    }

    close(sockfd);
    return 0;
}
