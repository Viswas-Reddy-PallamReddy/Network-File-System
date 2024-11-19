# Makefile for compiling NamingServer.c and trie.c

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -pthread

# Target executable
TARGET = NamingServer

# Source files
SOURCES = NamingServer.c trie.c LRU.c


# Object files (generated from sources)
OBJECTS = $(SOURCES:.c=.o)

# Default rule
all: $(TARGET)

# Rule to create the target
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

# Rule for generating object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJECTS) $(TARGET)
