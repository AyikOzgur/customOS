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

#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>


#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

void mountRootFileSystem()
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
        perror("/mnt could not created.");

    // Attempt to mount the device
    if (mount(source, target, filesystemtype, mountflags, data) != 0)
        perror("ext4 partition could not mounted to /mnt");
    else
        printf("Mounted %s to %s\n", source, target);

    // Change root
    if (chroot("/mnt") < 0)
    {
        perror("Root file system could not switched.");
    }
    else
    {
        printf("Root file system switched.\n");
        chdir("/");
    }

    // Attempt to mount the /proc filesystem
    if (mount("proc", "/proc", "proc", 0, NULL) != 0)
        perror("Error mounting /proc");
    else
        printf("proc mounted.");

    // Attempt to mount the devtmpfs on /dev
    if (mount("devtmpfs", "/dev", "devtmpfs", 0, NULL) != 0)
        perror("Error mounting /dev");
    else
        printf("/dev has been successfully mounted.\n");

    // Attempt to mount the sysfs on /sys
    if (mount("sysfs", "/sys", "sysfs", 0, NULL) != 0)
        perror("Error mounting /sys\n");
    else
        printf("/sys has been successfully mounted.\n");
}

void mountBootPartition()
{
    const char *source = "/dev/mmcblk0p1";
    const char *target = "/boot";
    const char *filesystemtype = "vfat";
    unsigned long mountflags = 0;
    const void *data = NULL;
    sleep(1);

    // Attempt to mount the device
    if (mount(source, target, filesystemtype, mountflags, data) != 0)
        perror("fat32 partition could not mounted to /boot");
    else
        printf("Mounted %s to %s\n", source, target);
}

void setupEth0()
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        perror("Error opening socket for eth0");

    struct ifreq ifr;
    strcpy(ifr.ifr_name, "eth0");

    struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;
    addr->sin_family = AF_INET;

    if (inet_pton(AF_INET, "192.168.0.2", &addr->sin_addr) != 1)
    {
        perror("Error setting IP address");
        close(fd);
    }
    if (ioctl(fd, SIOCSIFADDR, &ifr) < 0)
    {
        perror("Error setting interface address");
        close(fd);
    }

    if (inet_pton(AF_INET, "255.255.255.0", &addr->sin_addr) != 1)
    {
        perror("Error setting netmask\n");
        close(fd);
    }
    if (ioctl(fd, SIOCSIFNETMASK, &ifr) < 0)
    {
        perror("Error setting interface netmask");
        close(fd);
    }

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0)
    {
        perror("Error getting interface flags");
        close(fd);
    }

    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
    {
        perror("Error setting interface flags for eth");
        close(fd);
    }

    close(fd);
}

void setupLo()
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        perror("Error opening socket for loopback device");

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, "lo");

    struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;
    addr->sin_family = AF_INET;
    if (inet_pton(AF_INET, "127.0.0.1", &addr->sin_addr) != 1)
    {
        perror("Error setting IP address for lo\n");
        close(fd);
    }

    if (ioctl(fd, SIOCSIFADDR, &ifr) < 0)
    {
        perror("Error setting interface address for lo");
        close(fd);
    }

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0)
    {
        perror("Error getting interface flags for lo");
        close(fd);
    }

    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
    {
        perror("Error setting interface flags for lo");
        close(fd);
    }

    close(fd);
}

int main()
{
    mountRootFileSystem();
    mountBootPartition();
    setupEth0();
    setupLo();

    /* It will be enough for now to have only /lib for dynamic linker
        since we dont have anything except libc.
    */
    const char *libraryPath = "/lib";
    char ldLibraryPath[1024];
    snprintf(ldLibraryPath, sizeof(ldLibraryPath), "LD_LIBRARY_PATH=%s", libraryPath);
    if (putenv(ldLibraryPath) != 0)
        perror("LD_LIBRARY_PATH could not set.");

    while (1)
    {
        pid_t pid = fork();

        if (pid == -1)
        {
            perror("fork failed failed from init.... It will be tried again in 1 second");
            sleep(1);
            continue;
        }
        else if (pid == 0)
        {
            char *path = "/bin/remoteShell";
            char *argv[] = {path, NULL};
            char *envp[] = {NULL};

            // Note: execve does not return on success, the current program is replaced
            int status = execve(path, argv, envp);
            if (status == -1)
                perror("Cant run remoteShell");

            // Child process that runs remoteShell should not reach here.
            sleep(1);
            exit(0); // Terminate the child process.
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);
        }
    }

    return 0;
}