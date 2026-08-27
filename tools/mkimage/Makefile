SRC_DIR = .
BUILD_DIR = ../../build/tools/mkimage
BIN_DIR = ../../bin

TARGET = $(BUILD_DIR)/mkimage
DISK_IMG := $(BIN_DIR)/disk.img

MBR_BIN := ../../build/bootloader/mbr.bin
VBR_BIN := ../../build/bootloader/fat32_vbr.bin
DISK_FILES := ../../config/disk.files

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))


CC = gcc
CFLAGS = -Wall -Wextra


all: $(TARGET)


$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(OBJ_FILES) -o $@


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

image: $(TARGET)
	@mkdir -p $(BIN_DIR)
	$(TARGET) $(MBR_BIN) $(VBR_BIN) $(DISK_IMG) $(DISK_FILES)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(DISK_IMG)


-include $(OBJ_FILES:.o=.d)