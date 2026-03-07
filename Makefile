CC      = gcc
CFLAGS  = -std=c99 -IIncludes
SRC_DIR = Sources
OBJ_DIR = Objects

# Find all .c files in Sources/
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Convert Sources/foo.c → Objects/foo.o
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET = program

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile each .c into .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create Objects directory if missing
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean

