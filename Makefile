# Toolchain
ASM = nasm
CC  = i686-elf-gcc
LD  = i686-elf-ld

# Project Structure
LINKER_SCRIPT = linker.ld
BIN_DIR       = ./bin
BUILD_DIR     = ./build
SRC_DIR       = ./src
KERNEL_SRC_DIR = $(SRC_DIR)/kernel

# Compilation Flags
FLAGS = -g -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -I$(KERNEL_SRC_DIR)

KERNEL_ENTRY_SRC = $(SRC_DIR)/kernel.asm
KERNEL_ENTRY_OBJ = $(BUILD_DIR)/kernel.asm.o

C_SOURCES   := $(shell find $(SRC_DIR) -name "*.c")
ASM_SOURCES := $(shell find $(SRC_DIR) -name "*.asm" ! -name "kernel.asm" ! -path "$(SRC_DIR)/boot/*")

C_OBJECTS   := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
ASM_OBJECTS := $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.asm.o, $(ASM_SOURCES))

# Object Files
FILES = $(KERNEL_ENTRY_OBJ) $(ASM_OBJECTS) $(C_OBJECTS)

.PHONY: all clean

all: $(BIN_DIR)/os.bin
	@echo "--- [IwomhOS] Compilation completed successfully ---"

# Merge bootloader and kernel binaries, then truncate the image to 50K
$(BIN_DIR)/os.bin: $(BIN_DIR)/boot.bin $(BIN_DIR)/kernel.bin
	@mkdir -p $(BIN_DIR)
	dd if=$(BIN_DIR)/boot.bin of=$@ bs=512 conv=notrunc 2>/dev/null
	dd if=$(BIN_DIR)/kernel.bin of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null
	truncate -s 50K $@

# Compile the Bootloader (16-bit Assembly)
$(BIN_DIR)/boot.bin: $(SRC_DIR)/boot/boot.asm
	@mkdir -p $(BIN_DIR)
	$(ASM) -f bin $< -o $@

# Compile the Kernel entry point (32-bit Assembly)
$(KERNEL_ENTRY_OBJ): $(KERNEL_ENTRY_SRC)
	@mkdir -p $(BUILD_DIR)
	$(ASM) -f elf -g $< -o $@

# Generic .asm -> .asm.o
$(BUILD_DIR)/%.asm.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(ASM) -f elf -g $< -o $@

# Generic .c -> .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
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
	@echo "--- [IwomhOS] Build and bin directories cleaned ---"