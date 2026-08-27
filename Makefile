ARCH ?= x86
BUILD ?= debug

include config/default.mk
include config/arch/$(ARCH).mk

BIN_DIR := bin
DISK_IMG := bin/disk.img
KERNEL_ELF := build/kernel/kernel.elf

# Default target
.PHONY: all boot kernel tools clean run debug gdb

all: image
	@printf "\n--- [IWOS] Build complete ---\n"

boot:
	@printf "\n--- [IWOS] Building bootloader ---\n"
	$(MAKE) -C bootloader ARCH=$(ARCH)

kernel:
	@printf "\n--- [IWOS] Building kernel ---\n"
	$(MAKE) -C kernel ARCH=$(ARCH) BUILD=$(BUILD)

tools:
	@printf "\n--- [IWOS] Building host tools ---\n"
	$(MAKE) -C tools/mkimage

image: boot kernel tools
	@printf "\n--- [IWOS] Creating disk image ---\n"
	$(MAKE) -C tools/mkimage image


# Utilities
clean:
	$(MAKE) -C bootloader ARCH=$(ARCH) clean
	$(MAKE) -C kernel ARCH=$(ARCH) clean
	$(MAKE) -C tools/mkimage ARCH=$(ARCH) clean

	rm -rf $(BIN_DIR)

	@echo "\n--- [IWOS] Clean complete ---\n"

run: image
	qemu-system-i386 -drive file=$(DISK_IMG),format=raw

debug: image	
	qemu-system-i386 -drive file=$(DISK_IMG),format=raw -s -S

gdb: kernel
	$(GDB) $(KERNEL_ELF) -x scripts/kernel.gdb
