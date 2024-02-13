#include <stdio.h>

int main(int argc, char *argv[]) {
    FILE *file;
    int c;

    // Iterate over command line arguments (file paths)
    for (int i = 1; i < argc; i++) {
        // Open the file
        file = fopen(argv[i], "r");
        if (file == NULL) {
            printf("cat: %s: No such file or directory\n", argv[i]);
            continue; // Move to the next file if this one couldn't be opened
        }

        // Read and print the contents of the file
        while ((c = fgetc(file)) != EOF) {
            putchar(c);
        }

        // Close the file
        fclose(file);
    }

    return 0;
}
