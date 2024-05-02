#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Function to read and return total CPU usage
double get_total_cpu_usage() {
    FILE *stat_file = fopen("/proc/stat", "r");
    if (!stat_file) {
        perror("Error opening /proc/stat");
        exit(EXIT_FAILURE);
    }

    char line[256];
    fgets(line, sizeof(line), stat_file); // Read first line containing total CPU usage
    fclose(stat_file);

    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", 
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);

    // Calculate total CPU time
    unsigned long long total_cpu_time = user + nice + system + idle + iowait + irq + softirq + steal;

    // Calculate total CPU usage as a percentage
    double total_cpu_usage = (double)(total_cpu_time - idle) / total_cpu_time * 100.0;

    return total_cpu_usage;
}

// Function to read and return total memory usage in kilobytes
unsigned long long get_total_memory_usage() {
    FILE *meminfo_file = fopen("/proc/meminfo", "r");
    if (!meminfo_file) {
        perror("Error opening /proc/meminfo");
        exit(EXIT_FAILURE);
    }

    char line[256];
    unsigned long long mem_total, mem_free, buffers, cached;
    while (fgets(line, sizeof(line), meminfo_file)) {
        if (sscanf(line, "MemTotal: %llu kB", &mem_total) == 1) {}
        else if (sscanf(line, "MemFree: %llu kB", &mem_free) == 1) {}
        else if (sscanf(line, "Buffers: %llu kB", &buffers) == 1) {}
        else if (sscanf(line, "Cached: %llu kB", &cached) == 1) {}
    }
    fclose(meminfo_file);

    // Calculate total memory usage in kilobytes
    unsigned long long total_memory_usage = mem_total - mem_free - buffers - cached;

    return total_memory_usage;
}

int main() {
    double total_cpu_usage = get_total_cpu_usage();
    unsigned long long total_memory_usage = get_total_memory_usage();

    printf("Total CPU Usage: %.2f%%\n", total_cpu_usage);
    printf("Total Memory Usage: %llu kB\n", total_memory_usage);

    return 0;
}
