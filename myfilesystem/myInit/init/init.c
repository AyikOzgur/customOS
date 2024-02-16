#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

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

    // Attempt to mount the /proc filesystem
    if (mount("proc", "/proc", "proc", 0, NULL) != 0) {
        // If something went wrong, print out the error
        printf("Error mounting /proc:");
        //return EXIT_FAILURE;
    }
    else
    {
        printf("proc mounted.");
    }

    // Attempt to mount the devtmpfs on /dev
    if (mount("devtmpfs", "/dev", "devtmpfs", 0, NULL) != 0) {
        // If something went wrong, print out the error
        printf("Error mounting /dev:\n");
        //return EXIT_FAILURE;
    }
    else
        printf("/dev has been successfully mounted.\n");

    // Attempt to mount the sysfs on /sys
    if (mount("sysfs", "/sys", "sysfs", 0, NULL) != 0) {
        // If something went wrong, print out the error
        printf("Error mounting /sys\n");
        //return EXIT_FAILURE;
    }
    else
        printf("/sys has been successfully mounted.\n");

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


void setup_network() {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("Error opening socket");
        exit(EXIT_FAILURE);
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name, "eth0");

    struct sockaddr_in* addr = (struct sockaddr_in*)&ifr.ifr_addr;
    addr->sin_family = AF_INET;

    if (inet_pton(AF_INET, "192.168.0.2", &addr->sin_addr) != 1) {
        perror("Error setting IP address");
        close(fd);
        exit(EXIT_FAILURE);
    }
    if (ioctl(fd, SIOCSIFADDR, &ifr) < 0) {
        perror("Error setting interface address");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (inet_pton(AF_INET, "255.255.255.0", &addr->sin_addr) != 1) {
        perror("Error setting netmask");
        close(fd);
        exit(EXIT_FAILURE);
    }
    if (ioctl(fd, SIOCSIFNETMASK, &ifr) < 0) {
        perror("Error setting interface netmask");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        perror("Error getting interface flags");
        close(fd);
        exit(EXIT_FAILURE);
    }
    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
        perror("Error setting interface flags");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
}


void setup_loopback() {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("Error opening socket");
        exit(EXIT_FAILURE);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, "lo");

    struct sockaddr_in* addr = (struct sockaddr_in*)&ifr.ifr_addr;
    addr->sin_family = AF_INET;
    if (inet_pton(AF_INET, "127.0.0.1", &addr->sin_addr) != 1) {
        perror("Error setting IP address for lo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (ioctl(fd, SIOCSIFADDR, &ifr) < 0) {
        perror("Error setting interface address for lo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        perror("Error getting interface flags for lo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
        perror("Error setting interface flags for lo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
}

int main() 
{
    mntrootFilePart();
    mntbootPart();
    setup_network();
    setup_loopback();
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
        pid_t pid = fork();

        if (pid == -1) 
        {
            perror("fork");
            return 1;
        } 
        else if (pid == 0) 
        {
            // Child process
            // Check if the executable is in /bin
            // Path to the executable you want to run
            char *path = "/bin/myShell";

            // Arguments array for execve must be terminated by a NULL pointer
            // Since your executable doesn't require arguments, only include the program name
            char *argv[] = {path, NULL};

            // Environment variables array, also terminated by a NULL pointer
            // If you don't need to set any specific environment variables, you can just pass NULL
            char *envp[] = {NULL};

            // Use execve to execute the program
            // Note: execve does not return on success, the current program is replaced
            int status = execve(path, argv, envp);
            if (status == -1) 
            {
                perror("execve failed");
            }
            printf("this message should not be seen");

            exit(0); // Terminate the child process
        } 
        else 
        {
            // Parent process
            int status;
            waitpid(pid, &status, 0);
            // The parent continues to run after the child process finishes.
        }
    }

    return 0;
}
