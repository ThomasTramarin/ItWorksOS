include ../config/default.mk
include ../config/arch/$(ARCH).mk

SYSTEM_ASM_SRC := system.asm
SYSTEM_IWBF    := ../build/user/system.iwbf

ifeq ($(ARCH),x86)

$(SYSTEM_IWBF): $(SYSTEM_ASM_SRC)
	@mkdir -p $(dir $@)
	$(AS) -f bin $< -o $@

endif