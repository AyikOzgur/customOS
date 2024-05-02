#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

// Experimental ports, they should be updated.
#define SEND_PORT 7002 // Note: This should match the receiver's RECV_PORT
#define RECV_PORT 7001 // For any acknowledgments, if applicable

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* filePath = argv[1]; // Get the file path from the command line arguments

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
    myAddr.sin_addr.s_addr = inet_addr("192.168.0.3");

    if (bind(sockfd, (struct sockaddr *)&myAddr, sizeof(myAddr)) < 0) 
    {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(SEND_PORT);
    destAddr.sin_addr.s_addr = inet_addr("192.168.0.2");

    int fileFd = open(filePath, O_RDONLY);
    if (fileFd < 0) 
    {
        perror("Failed to open the file");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Get file size
    off_t fileSize = lseek(fileFd, 0, SEEK_END);
    lseek(fileFd, 0, SEEK_SET); // Reset file descriptor position.

    // Prepare the message with size and name
    char fileInfo[1024]; // Ensure this buffer is big enough
    int fileInfoLen = snprintf(fileInfo, sizeof(fileInfo), "%ld:%s", (long)fileSize, filePath) + 1; // +1 for null terminato
    // Send file info (size and name)
    if (sendto(sockfd, fileInfo, fileInfoLen, 0, (struct sockaddr *)&destAddr, destAddrLen) < 0) 
    {
        perror("Failed to send file info");
        close(fileFd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Await acknowledgment (simple protocol)
    char ackBuffer[1024];
    ssize_t ackLen = recvfrom(sockfd, ackBuffer, sizeof(ackBuffer) - 1, 0, NULL, NULL); // Leave space for null-terminator
    if (ackLen < 0) 
    {
        perror("Failed to receive acknowledgment");
        close(fileFd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Null-terminate the received data to safely perform string operations
    ackBuffer[ackLen] = '\0';

    // Check if the received acknowledgment is "okay"
    if (strncmp(ackBuffer, "okay", ackLen) == 0) 
    {
        printf("Acknowledgment 'okay' received.\n");
    } 
    else 
    {
        printf("Unexpected acknowledgment received.\n");
        close(fileFd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Acknowledgment received, now send the file content
    lseek(fileFd, 0, SEEK_SET); // Ensure we're at the start of the file
    unsigned char* fileBuffer = malloc(fileSize);
    if (!fileBuffer) 
    {
        perror("Failed to allocate memory for file content");
        close(fileFd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (read(fileFd, fileBuffer, fileSize) != fileSize) 
    {
        perror("Failed to read the file");
        free(fileBuffer);
        close(fileFd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Calculate how many full buffers we will send.
    int numberofBuffer = (int)fileSize / 1400;

    // Send full packets.
    for(int i = 0; i < numberofBuffer; ++i)
    {  
        // Send the actual file data.
        if (sendto(sockfd, &fileBuffer[i * 1400], 1400, 0, (struct sockaddr *)&destAddr, destAddrLen) < 0) 
        {
            perror("Failed to send file data");
            free(fileBuffer);
            close(fileFd);
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // Give receiver some time to get and wait new one.
        sleep(1);
    }

    // Send last packet with size less than 1400.
    int lastPacketSize = fileSize % 1400;
    // Send the actual file data
    if (sendto(sockfd, &fileBuffer[numberofBuffer * 1400], lastPacketSize, 0, (struct sockaddr *)&destAddr, destAddrLen) < 0) 
    {
        perror("Failed to send file data");
        free(fileBuffer);
        close(fileFd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("File '%s' sent successfully.\n", filePath);

    // Cleanup.
    free(fileBuffer);
    close(fileFd);
    close(sockfd);

    return 0;
}