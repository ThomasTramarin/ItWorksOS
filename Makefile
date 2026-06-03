# Toolchain
ASM = nasm
CC  = i686-elf-gcc
LD  = i686-elf-ld

# Project Structure
LINKER_SCRIPT = linker.ld
BIN_DIR       = ./bin
BUILD_DIR     = ./build
SRC_DIR       = ./src

# Compilation Flags
FLAGS = -g -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -I$(SRC_DIR)/kernel

# Object Files
FILES = $(BUILD_DIR)/kernel.asm.o $(BUILD_DIR)/kernel.o

.PHONY: all clean

all: $(BIN_DIR)/os.bin
	@echo "--- [WomhOS] Compilation completed successfully ---"

# 1. Merge bootloader and kernel binaries, then truncate the image to 50K
$(BIN_DIR)/os.bin: $(BIN_DIR)/boot.bin $(BIN_DIR)/kernel.bin
	@mkdir -p $(BIN_DIR)
	dd if=$(BIN_DIR)/boot.bin of=$@ bs=512 conv=notrunc 2>/dev/null
	dd if=$(BIN_DIR)/kernel.bin of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null
	truncate -s 50K $@

# 2. Compile the Bootloader (16-bit Assembly)
$(BIN_DIR)/boot.bin: $(SRC_DIR)/boot/boot.asm
	@mkdir -p $(BIN_DIR)
	$(ASM) -f bin $< -o $@

# 3. Compile the Kernel entry point (32-bit Assembly)
$(BUILD_DIR)/kernel.asm.o: $(SRC_DIR)/kernel.asm
	@mkdir -p $(BUILD_DIR)
	$(ASM) -f elf -g $< -o $@

# 4. Compile the Kernel core (C)
$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel/kernel.c $(SRC_DIR)/kernel/kernel.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(FLAGS) -std=gnu99 -c $< -o $@

# 5. Relocatable link (merges Kernel Assembly and C objects)
$(BUILD_DIR)/completeKernel.o: $(FILES)
	$(LD) -g -relocatable $(FILES) -o $@

# 6. Final link phase using the linker script
$(BIN_DIR)/kernel.bin: $(BUILD_DIR)/completeKernel.o $(LINKER_SCRIPT)
	$(CC) $(FLAGS) -T $(LINKER_SCRIPT) -o $@ -ffreestanding -O0 -nostdlib $<

# CLEANUP
clean:
	rm -rf $(BIN_DIR)/*
	rm -rf $(BUILD_DIR)/*
	@echo "--- [WomhOS] Build and bin directories cleaned ---"