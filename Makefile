CC = gcc

MODE ?= debug

# DEBUG_FLAGS   = -std=c99 -Wall -Wextra -Wshadow -Wpointer-arith \
                # -Wstrict-prototypes -Wstrict-overflow=5 -Wcast-qual \
                # -Wwrite-strings -Wconversion -pedantic -g -O0
DEBUG_FLAGS = -std=c99 -g -O0

# RELEASE_FLAGS = -std=c99 -Wall -Wextra -O2 -DNDEBUG
RELEASE_FLAGS = -std=c99 -O2 -DNDEBUG

CFLAGS = $(if $(filter $(MODE),debug),$(DEBUG_FLAGS),$(RELEASE_FLAGS)) -IIncludes

SRC_DIR = Sources
OBJ_DIR = Objects

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET = program

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

run: $(TARGET)
	./$(TARGET)

valgrind: MODE=debug
valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all \
			--track-origins=yes --verbose ./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean run valgrind
