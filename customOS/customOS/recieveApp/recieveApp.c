#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// Experimental ports, they should be updated.
#define SEND_PORT 7001
#define RECV_PORT 7002

int main() 
{
    int sockfd;
    struct sockaddr_in myAddr, destAddr;
    socklen_t destAddrLen = sizeof(destAddr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    {
        perror("Sender socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&myAddr, 0, sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(RECV_PORT);
    myAddr.sin_addr.s_addr = inet_addr("192.168.0.2");

    if (bind(sockfd, (struct sockaddr *)&myAddr, sizeof(myAddr)) < 0) 
    {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(SEND_PORT);
    destAddr.sin_addr.s_addr = inet_addr("192.168.0.3");

    int bufferSize = 1024;
    uint8_t* buffer = (uint8_t*)malloc(bufferSize);
    if (!buffer) 
    {
        perror("Memory allocation failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int receivedBytes = recvfrom(sockfd, buffer, bufferSize, 0, (struct sockaddr *)&destAddr, &destAddrLen);
    if (receivedBytes < 0) 
    {
        perror("Receive failed");
        free(buffer);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Assuming buffer contains the string "123:appExecutableName"
    int sizeofExecutable;
    char* colonPtr = strchr(buffer, ':'); // Find the colon in the buffer
    if (colonPtr == NULL) 
    {
        perror("Format error: No colon found");
        exit(EXIT_FAILURE);
    }

    *colonPtr = '\0'; // Temporarily terminate the string at the colon for atoi
    sizeofExecutable = atoi(buffer); // Convert the size part to integer
    char* nameOfExecutable = colonPtr + 1; // The name starts right after the colon

    // Print the size and name of the executable
    printf("Size of executable: %d bytes\n", sizeofExecutable);
    printf("Name of executable: %s\n", nameOfExecutable);

    const char* ackMessage = "okay";
    sendto(sockfd, ackMessage, strlen(ackMessage), 0, (struct sockaddr *)&destAddr, destAddrLen);

    uint8_t* executableData = malloc(sizeofExecutable);
    if (!executableData) 
    {
        perror("Memory allocation failed for executable data");
        free(buffer);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Calculate number of buffers.
    int numberofBuffers = sizeofExecutable / 1400;
    
    // Get full packets.
    for(int i = 0; i < numberofBuffers; ++i)
    {
        int bytes = recvfrom(sockfd, &executableData[i * 1400], 1400, 0, (struct sockaddr *)&destAddr, &destAddrLen);
        if (bytes != 1400) 
        {
            perror("Executable data receive failed or incomplete");
            free(executableData);
            free(buffer);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }

    //  Get last package with non ful size.
    int lastPacketSize = sizeofExecutable % 1400;
    int bytes = recvfrom(sockfd, &executableData[numberofBuffers * 1400], lastPacketSize, 0, (struct sockaddr *)&destAddr, &destAddrLen);
    if (bytes != lastPacketSize) 
    {
        perror("Executable data receive failed or incomplete");
        free(executableData);
        free(buffer);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int fileFd = open(nameOfExecutable, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    if (fileFd < 0) 
    {
        perror("File open failed");
        free(executableData);
        free(buffer);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if (write(fileFd, executableData, sizeofExecutable) != sizeofExecutable) 
    {
        perror("Failed to write executable data to file");
        close(fileFd);
        free(executableData);
        free(buffer);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    close(fileFd);

    printf("Executable '%s' received and saved.\n", nameOfExecutable);

    free(executableData);
    free(buffer);
    close(sockfd);

    return 0;
}