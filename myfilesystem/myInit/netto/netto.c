#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SEND_PORT 7031
#define RECV_PORT 7032

void startReceiver()
{
    int sockfd;
    struct sockaddr_in recvAddr, senderAddr;
    char buffer[1024];
    socklen_t len;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("Receiver socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&recvAddr, 0, sizeof(recvAddr));
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_addr.s_addr = inet_addr("192.168.0.2");
    recvAddr.sin_port = htons(RECV_PORT);

    if (bind(sockfd, (struct sockaddr *)&recvAddr, sizeof(recvAddr)) < 0)
    {
        printf("Receiver bind failed");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        len = sizeof(senderAddr);
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&senderAddr, &len);
        if (n < 0)
        {
            printf("Receiver error");
            continue;
        }
        buffer[n] = '\0';
        printf("Received: %s\n", buffer);
    }

    close(sockfd);
}

void startSender()
{
    int sockfd;
    struct sockaddr_in destAddr;
    char message[] = "Hello World";

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("Sender socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(RECV_PORT);
    destAddr.sin_addr.s_addr = inet_addr("192.168.0.2");

    while (1)
    {
        sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
        printf("Message sent\n");
        sleep(1);
    }

    close(sockfd);
}

int main()
{
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        // Child process: Receiver
        startReceiver();
    }
    else
    {
        // Parent process: Sender
        startSender();
    }

    return 0;
}
