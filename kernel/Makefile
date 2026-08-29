ARCH ?= x86

include ../config/default.mk
include ../config/arch/$(ARCH).mk

CORE_DIR          := core
MM_DIR            := mm
IRQ_DIR 		  := irq
KLIB_DIR          := klib
DRIVERS_DIR       := drivers
INCLUDE_DIR       := include
ARCH_DIR          := arch/$(ARCH)
ARCH_INCLUDE_DIR  := $(ARCH_DIR)/include

BUILD_DIR := ../build/kernel

KERNEL_ELF 		:= $(BUILD_DIR)/kernel.elf
KERNEL_BIN 		:= $(BUILD_DIR)/kernel.bin

KERNEL_LDSCRIPT := $(ARCH_DIR)/linker.ld

CFLAGS = \
		-ffreestanding \
		-nostdlib \
		-nostartfiles \
		-nodefaultlibs \
		-Wall \
		-Wextra \
		-std=gnu99 \
		-I$(INCLUDE_DIR) \
		-I$(ARCH_INCLUDE_DIR) \

ifeq ($(BUILD),debug)
	CFLAGS += -g3 -O0
else ifeq ($(BUILD),release)
	CFLAGS += -O2
endif

# Kernel Entry
KERNEL_ENTRY_SRC := $(ARCH_DIR)/kernel.asm
KERNEL_ENTRY_OBJ := $(BUILD_DIR)/arch/$(ARCH)/kernel.asm.o

# Source Discovery
C_SOURCES := \
	$(shell find \
		$(CORE_DIR) \
		$(MM_DIR) \
		$(IRQ_DIR) \
		$(KLIB_DIR) \
		$(DRIVERS_DIR) \
		-name '*.c' \
	)

ARCH_C_SOURCES := \
	$(shell find $(ARCH_DIR) \
	-name '*.c' \
	)

ASM_SOURCES := \
	$(shell find $(ARCH_DIR) -name '*.asm' \
		! -name 'kernel.asm')

# Object Files
C_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))

ARCH_C_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(ARCH_C_SOURCES))

ASM_OBJECTS := $(patsubst %.asm,$(BUILD_DIR)/%.asm.o,$(ASM_SOURCES))

KERNEL_OBJECTS := $(KERNEL_ENTRY_OBJ) $(C_OBJECTS) $(ARCH_C_OBJECTS) $(ASM_OBJECTS)

.PHONY: all clean

all: $(KERNEL_BIN)

# Generate object files
$(KERNEL_ENTRY_OBJ): $(KERNEL_ENTRY_SRC)
	@mkdir -p $(dir $@)
	$(AS) -f elf32 -g $< -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.asm.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) -f elf32 -g $< -o $@

# Link Kernel
$(KERNEL_ELF): $(KERNEL_OBJECTS) $(KERNEL_LDSCRIPT)
	@mkdir -p $(dir $@)
	$(LD) -T $(KERNEL_LDSCRIPT) -o $@ $(KERNEL_OBJECTS)

# Extract flat kernel binary
$(KERNEL_BIN): $(KERNEL_ELF)
	@mkdir -p $(dir $@)
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)


-include $(C_OBJECTS:.o=.d)
-include $(ARCH_C_OBJECTS:.o=.d)