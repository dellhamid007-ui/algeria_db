# ============================================================
#  Algeria History Database – Makefile  (Linux / GCC)
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -g -std=c11 -Iinclude
TARGET  = algeria_db
SRC_DIR = src
OBJ_DIR = obj

SRCS = $(SRC_DIR)/main.c      \
       $(SRC_DIR)/utils.c     \
       $(SRC_DIR)/file_parser.c \
       $(SRC_DIR)/list.c      \
       $(SRC_DIR)/stack.c     \
       $(SRC_DIR)/tree.c      \
       $(SRC_DIR)/recursion.c

OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

.PHONY: all clean run

all: $(OBJ_DIR) $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Build successful: ./$(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
