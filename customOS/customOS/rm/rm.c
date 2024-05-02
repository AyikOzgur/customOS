#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

void remove_directory(const char *path) 
{
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        printf("Failed to open directory %s\n", path);
        return;
    }

    struct dirent *entry;
    char full_path[1024];

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            
            struct stat statbuf;
            if (stat(full_path, &statbuf) != 0)
            {
                printf("Failed to get file status %s\n", full_path);
                continue;
            }

            if (S_ISDIR(statbuf.st_mode))
            {
                remove_directory(full_path);
            } 
            else 
            {
                if (unlink(full_path) != 0)
                {
                    printf("Failed to remove file %s\n", full_path);
                }
            }
        }
    }

    closedir(dir);

    if (rmdir(path) != 0) 
    {
        printf("Failed to remove directory %s\n",path);
    }
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        printf("Usage: %s <path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct stat statbuf;
    if (stat(argv[1], &statbuf) != 0) 
    {
        printf( "Error accessing %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if (S_ISDIR(statbuf.st_mode)) 
    {
        remove_directory(argv[1]);
    } 
    else 
    {
        if (unlink(argv[1]) != 0) 
        {
            printf("Failed to remove file %s\n", argv[1]);
        }
    }

    return 0;
}