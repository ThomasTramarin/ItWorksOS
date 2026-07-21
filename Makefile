# Toolchain
ASM = nasm
CC  = i686-elf-gcc
LD  = i686-elf-ld

# Project Structure
LINKER_SCRIPT  = linker.ld
BIN_DIR        = ./bin
BUILD_DIR      = ./build
SRC_DIR        = ./src
INCLUDE_DIR    = ./include
TOOLS_DIR      = ./tools
MKIMAGE_DIR = $(TOOLS_DIR)/mkimage
MKIMAGE_BIN = $(BUILD_DIR)/tools/mkimage/mkimage
BOOT_SRC_DIR   = $(SRC_DIR)/boot/x86
KERNEL_SRC_DIR = $(SRC_DIR)/kernel
FILES_LIST = ./disk.files

# Target Outputs
DISK_IMG 	= $(BIN_DIR)/disk.img
KERNEL_BIN 	= $(BIN_DIR)/kernel.bin
MBR_BIN    	= $(BUILD_DIR)/mbr.bin
VBR_BIN    	= $(BUILD_DIR)/fat32_vbr.bin
STAGE2_BIN  = $(BUILD_DIR)/stage2.bin

# Compilation Flags
FLAGS = -g -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -I$(KERNEL_SRC_DIR) -I$(INCLUDE_DIR)

# Kernel Entry
KERNEL_ENTRY_SRC = $(SRC_DIR)/kernel/arch/x86/kernel.asm
KERNEL_ENTRY_OBJ = $(BUILD_DIR)/kernel/arch/x86/kernel.asm.o

# Automatic src discovery (isolates kernel from boot)
C_SOURCES   := $(shell find $(SRC_DIR) -name "*.c")
ASM_SOURCES := $(shell find $(KERNEL_SRC_DIR) -name "*.asm" ! -name "kernel.asm")

C_OBJECTS   := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
ASM_OBJECTS := $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.asm.o, $(ASM_SOURCES))

KERNEL_FILES = $(KERNEL_ENTRY_OBJ) $(ASM_OBJECTS) $(C_OBJECTS)

.PHONY: all clean run

all: $(MKIMAGE_BIN) $(DISK_IMG)
	@echo "--- [IwomhOS] Disk image created successfully ---"

# --- HOST TOOLS ---
$(MKIMAGE_BIN):
	@echo "Building mkimage..."
	$(MAKE) -C $(MKIMAGE_DIR)

# --- BOOTLOADER RULES ---
NASM_FLAGS = -I$(BOOT_SRC_DIR)/ -I$(BOOT_SRC_DIR)/mbr/ -I$(BOOT_SRC_DIR)/vbr/ -I$(BOOT_SRC_DIR)/stage2/

$(MBR_BIN): $(BOOT_SRC_DIR)/mbr/mbr.asm
	@mkdir -p $(dir $@)
	@echo "Assembling MBR"
	$(ASM) -f bin -g $(NASM_FLAGS) $< -o $@

$(VBR_BIN): $(BOOT_SRC_DIR)/vbr/fat32_vbr.asm
		@mkdir -p $(dir $@)
		@echo "Assembling VBR"
		$(ASM) -f bin -g $(NASM_FLAGS) $< -o $@

$(STAGE2_BIN): $(BOOT_SRC_DIR)/stage2/stage2.asm
		@mkdir -p $(dir $@)
		@echo "Assembling Stage 2"
		$(ASM) -f bin -g $(NASM_FLAGS) $< -o $@

# --- KERNEL BUILD RULES ---

# Compile the Kernel entry point (32-bit Assembly)
$(KERNEL_ENTRY_OBJ): $(KERNEL_ENTRY_SRC)
	@mkdir -p $(dir $@)
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
$(BUILD_DIR)/completeKernel.o: $(KERNEL_FILES)
	@mkdir -p $(dir $@)
	$(LD) -g -relocatable $(KERNEL_FILES) -o $@

# 6. Final link phase using the linker script
$(BIN_DIR)/kernel.bin: $(BUILD_DIR)/completeKernel.o $(LINKER_SCRIPT)
	@mkdir -p $(dir $@)
	@echo "Linking Kernel..."
	$(CC) $(FLAGS) -T linker.ld -o $@ -ffreestanding -O0 -nostdlib $<

# --- DISK IMAGE GENERATION ---
$(DISK_IMG): $(MBR_BIN) $(VBR_BIN) $(STAGE2_BIN)
	@mkdir -p $(BIN_DIR)
	@echo "Generating disk image..."

	$(MKIMAGE_BIN) \
		$(MBR_BIN) \
		$(VBR_BIN) \
		$(DISK_IMG) \
		$(FILES_LIST)

# --- UTILITIES ---
clean:
	rm -rf $(BIN_DIR)/*
	rm -rf $(BUILD_DIR)/*
	@echo "--- [IwomhOS] Cleaned build environment ---"

run: all
	qemu-system-i386 -hda $(DISK_IMG)