# Compiler
CC = gcc

# Compiler flags (POSIX.1-2008, warnings as errors)
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -Iinclude

# Directories
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

# Sources and objects
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Final executable
TARGET = shell.out

# Default target
all: $(TARGET)

# Link object files into final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile .c into .o inside obj/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure obj/ exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Remove build artifacts
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Run the program after building
run: all
	./$(TARGET)
