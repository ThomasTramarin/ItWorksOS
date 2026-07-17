SRC_DIR = .
BUILD_DIR = ../../build/tools/mkimage

TARGET = $(BUILD_DIR)/mkimage

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))


CC = gcc
CFLAGS = -Iinclude -Wall -Wextra


all: $(TARGET)


$(TARGET): $(OBJ_FILES)
	mkdir -p $(BUILD_DIR)
	$(CC) $(OBJ_FILES) -o $@


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf $(BUILD_DIR)