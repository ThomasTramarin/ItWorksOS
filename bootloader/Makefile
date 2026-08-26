ARCH ?= x86

include ../config/default.mk
include ../config/arch/$(ARCH).mk

ASFLAGS = -f bin \
	-I$(ARCH)/ \
	-I$(ARCH)/mbr/ \
	-I$(ARCH)/vbr/ \
	-I$(ARCH)/stage2/


BUILD = ../build/bootloader

MBR_BIN := $(BUILD)/mbr.bin
VBR_BIN := $(BUILD)/fat32_vbr.bin
STAGE2_BIN := $(BUILD)/stage2.bin

.PHONY: all clean

all: $(MBR_BIN) $(VBR_BIN) $(STAGE2_BIN)


$(MBR_BIN): $(ARCH)/mbr/mbr.asm
	@mkdir -p $(dir $@)
	$(Q)$(AS) $(ASFLAGS) $< -o $@

$(VBR_BIN): $(ARCH)/vbr/fat32_vbr.asm
	@mkdir -p $(dir $@)
	$(Q)$(AS) $(ASFLAGS) $< -o $@

$(STAGE2_BIN): $(ARCH)/stage2/stage2.asm
	@mkdir -p $(dir $@)
	$(Q)$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf $(BUILD)