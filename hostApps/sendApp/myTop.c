#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define MAX_PROCESSES 100

struct ProcessInfo 
{
    int pid;
    double cpu_usage;
    unsigned long long memory_usage;
};

// Function to compare ProcessInfo structs by CPU usage
int compare_process_info(const void *a, const void *b) 
{
    const struct ProcessInfo *pa = (const struct ProcessInfo *)a;
    const struct ProcessInfo *pb = (const struct ProcessInfo *)b;
    if (pa->cpu_usage < pb->cpu_usage) return 1;
    if (pa->cpu_usage > pb->cpu_usage) return -1;
    return 0;
}

// Function to get CPU usage for a process
double get_process_cpu_usage(int pid) 
{
    // Implement your code to get CPU usage for the given PID
    return 0.0; // Placeholder
}

// Function to get memory usage for a process
unsigned long long get_process_memory_usage(int pid) 
{
    // Implement your code to get memory usage for the given PID
    return 0; // Placeholder
}

// Function to read process IDs from /proc directory
int read_proc_entries(int proc_dir_fd, char *buf, size_t buf_size) 
{
    ssize_t bytes_read = syscall(SYS_getdents, proc_dir_fd, buf, buf_size);
    if (bytes_read == -1) 
    {
        perror("Error reading /proc directory");
        close(proc_dir_fd);
        exit(EXIT_FAILURE);
    }
    return bytes_read;
}

int main() 
{
    struct ProcessInfo processes[MAX_PROCESSES];
    int num_processes = 0;

    // Open /proc directory
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) 
    {
        perror("Error opening /proc directory");
        exit(EXIT_FAILURE);
    }

    // Read process IDs from /proc directory
    struct dirent *dir_entry;
    while ((dir_entry = readdir(proc_dir)) != NULL && num_processes < MAX_PROCESSES) 
    {
        int pid = atoi(dir_entry->d_name);
        if (pid != 0) 
        {  
            // Skip special entries like "." and ".."
            double cpu_usage = get_process_cpu_usage(pid);
            unsigned long long memory_usage = get_process_memory_usage(pid);
            processes[num_processes].pid = pid;
            processes[num_processes].cpu_usage = cpu_usage;
            processes[num_processes].memory_usage = memory_usage;
            num_processes++;
        }
    }
    closedir(proc_dir);

    // Sort processes by CPU usage
    qsort(processes, num_processes, sizeof(struct ProcessInfo), compare_process_info);

    // Print process information
    printf("%-10s %-10s %-10s\n", "PID", "CPU(%)", "Memory(kB)");
    for (int i = 0; i < num_processes; i++) 
    {
        printf("%-10d %-10.2f %-10llu\n", processes[i].pid, processes[i].cpu_usage, processes[i].memory_usage);
    }

    return 0;
}