#include <stdio.h>

int main() {
    fprintf(stderr, "This is an error message to stderr.\n");
    return 1;  // Returning a non-zero value to indicate an error.
}
