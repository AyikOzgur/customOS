# Define compiler
CC=gcc

# Define compiler flags
CFLAGS=-Wall -Werror

# Define the target executable
TARGET=remoteTerminal

# Default target
all: $(TARGET)
	@rm -f *.o

# Rule to link the program
$(TARGET): remoteTerminal.o
	$(CC) $(CFLAGS) remoteTerminal.o -o $(TARGET)

# Rule to compile the source files
remoteTerminal.o: remoteTerminal.c
	$(CC) $(CFLAGS) -c remoteTerminal.c

# Target to clean the build files
clean:
	rm -f *.o $(TARGET)
