# Toolchain
ASM = nasm
CC  = i686-elf-gcc
LD  = i686-elf-ld

# Project Directories
SRC_DIR          = ./src
BUILD_DIR        = ./build
BIN_DIR          = ./bin
INCLUDE_DIR      = ./include
ARCH_INCLUDE_DIR = $(SRC_DIR)/arch/x86/include

TOOLS_DIR      = ./tools
MKIMAGE_DIR = $(TOOLS_DIR)/mkimage
MKIMAGE_BIN = $(BUILD_DIR)/tools/mkimage/mkimage

BOOT_DIR   = $(SRC_DIR)/boot/x86

# Output Files
KERNEL_BIN 	= $(BIN_DIR)/kernel.bin

DISK_IMG 	= $(BIN_DIR)/disk.img

MBR_BIN    	= $(BUILD_DIR)/mbr.bin
VBR_BIN    	= $(BUILD_DIR)/fat32_vbr.bin
STAGE2_BIN  = $(BUILD_DIR)/stage2.bin

# Kernel Entry Point
KERNEL_ENTRY_SRC := $(SRC_DIR)/arch/x86/kernel.asm
KERNEL_ENTRY_OBJ := $(BUILD_DIR)/arch/x86/kernel.asm.o

# Compiler Flags
CFLAGS = \
		-g \
		-ffreestanding \
		-nostdlib \
		-nostartfiles \
		-nodefaultlibs \
		-Wall \
		-O0 \
		-std=gnu99 \
		-I$(INCLUDE_DIR) \
		-I$(ARCH_INCLUDE_DIR) \
		-DARCH_X86

# Automatically discover kernel sources (to build the singce kernel.bin)
# Everything under src/ except bootloader sources belongs to the kernel source tree.

C_SOURCES := $(shell find $(SRC_DIR) \
	-path "$(BOOT_DIR)" -prune -o \
	-name "*.c" -print)

ASM_SOURCES := $(shell find $(SRC_DIR) \
	-path "$(BOOT_DIR)" -prune -o \
	-name "*.asm" \
	! -name "kernel.asm" -print)

C_OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))

ASM_OBJECTS := $(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.asm.o,$(ASM_SOURCES))

KERNEL_OBJECTS := \
	$(KERNEL_ENTRY_OBJ) \
	$(ASM_OBJECTS) \
	$(C_OBJECTS)

# Default target
.PHONY: all clean run

all: $(MKIMAGE_BIN) $(DISK_IMG)
	@echo "--- [IWOS] Disk image created successfully ---"

# Host tools
$(MKIMAGE_BIN):
	@echo "Building mkimage..."
	$(MAKE) -C $(MKIMAGE_DIR)

# Bootloader
NASM_BOOT_FLAGS = \
	-I$(BOOT_DIR)/ \
	-I$(BOOT_DIR)/mbr/ \
	-I$(BOOT_DIR)/vbr/ \
	-I$(BOOT_DIR)/stage2/



$(MBR_BIN): $(BOOT_DIR)/mbr/mbr.asm
	@mkdir -p $(dir $@)
	@echo "Assembling MBR..."
	$(ASM) -f bin -g $(NASM_BOOT_FLAGS) $< -o $@

$(VBR_BIN): $(BOOT_DIR)/vbr/fat32_vbr.asm
	@mkdir -p $(dir $@)
	@echo "Assembling VBR..."
	$(ASM) -f bin -g $(NASM_BOOT_FLAGS) $< -o $@

$(STAGE2_BIN): $(BOOT_DIR)/stage2/stage2.asm
	@mkdir -p $(dir $@)
	@echo "Assembling Stage2..."
	$(ASM) -f bin -g $(NASM_BOOT_FLAGS) $< -o $@


# Kernel entry assembly
$(KERNEL_ENTRY_OBJ): $(KERNEL_ENTRY_SRC)
	@mkdir -p $(dir $@)
	@echo "Assembling kernel entry..."
	$(ASM) -f elf -g $< -o $@

# Generic assembly compilation (.asm -> .asm.o)
$(BUILD_DIR)/%.asm.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(ASM) -f elf -g $< -o $@

# Generic C compilation (.c -> .o)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Merge all kernel objects
$(BUILD_DIR)/completeKernel.o: $(KERNEL_OBJECTS)
	@mkdir -p $(dir $@)
	@echo "Relocating kernel objects..."
	$(LD) -g -r $(KERNEL_OBJECTS) -o $@

# Final kernel binary
$(KERNEL_BIN): $(BUILD_DIR)/completeKernel.o linker.ld
	@mkdir -p $(dir $@)
	@echo "Linking kernel..."
	$(CC) \
		$(CFLAGS) \
		-T linker.ld \
		-o $@ \
		$<

# Disk image generation
$(DISK_IMG): $(MBR_BIN) $(VBR_BIN) $(STAGE2_BIN) $(KERNEL_BIN)
	@mkdir -p $(BIN_DIR)
	@echo "Generating disk image..."

	$(MKIMAGE_BIN) \
		$(MBR_BIN) \
		$(VBR_BIN) \
		$(DISK_IMG) \
		./disk.files

# Utilities
clean:
	rm -rf $(BIN_DIR)/*
	rm -rf $(BUILD_DIR)/*
	@echo "--- [IWOS] Cleaned build environment ---"

run: all
	qemu-system-i386 -hda $(DISK_IMG)

