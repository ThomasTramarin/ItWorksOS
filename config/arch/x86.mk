ARCH := x86

CROSS_PREFIX := i686-elf

CC := $(CROSS_PREFIX)-gcc
LD := $(CROSS_PREFIX)-ld
OBJCOPY := $(CROSS_PREFIX)-objcopy
GDB := $(CROSS_PREFIX)-gdb
STRIP := $(TARGET)-strip

AS := nasm