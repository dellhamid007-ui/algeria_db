# ============================================================
#  Algeria History Database – Makefile  (Linux / GCC)
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -g -std=c11 -Iinclude
TARGET  = algeria_db
GUI_TARGET = algeria_gui
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

# Shared modules (everything except main.c and gui.c)
SHARED_OBJS = $(OBJ_DIR)/utils.o $(OBJ_DIR)/file_parser.o \
              $(OBJ_DIR)/list.o $(OBJ_DIR)/stack.o \
              $(OBJ_DIR)/tree.o $(OBJ_DIR)/recursion.o

# Raylib flags (local install in lib/)
RAYLIB_FLAGS = -Llib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

.PHONY: all clean run gui run-gui terminal

all: terminal

terminal: $(OBJ_DIR) $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Build successful: ./$(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

gui: $(OBJ_DIR) $(GUI_TARGET)

$(GUI_TARGET): $(OBJ_DIR)/gui.o $(SHARED_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(RAYLIB_FLAGS)
	@echo "GUI build successful: ./$(GUI_TARGET)"

run: terminal
	./$(TARGET)

run-gui: gui
	LD_LIBRARY_PATH=lib ./$(GUI_TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(GUI_TARGET)
