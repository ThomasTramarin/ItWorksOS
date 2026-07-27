# Boot Protocol

The boot protocol defines the interface between the bootloader (stage2) and the kernel entry point.

## CPU state
At the moment of transferring execution to the kernel entry point, stage 2 leaves the CPU in the following state:
- **Execution Mode**: 32-bit Protected Mode
- **Interrupts**: disabled
- **Paging**: disabled
- **Segment Registers**: loaded with a GDT data descriptor (flat model)
- **Code Segment Register**: must point to a GDT code descriptor (flat model)
- **A20 Line**: enabled
- **Stack Pointer**: set to a valid and safe memory region in Stage2

The bootloader sets the following registers:

| Register | Value |
|----------|-------|
| EAX | BOOT_MAGIC |
| EBX | pointer to `boot_info` |

Then, it performs a far jump to the assembly code that will call the C kmain function.

## Boot Magic
Boot Magic is a 32-bit constant used by the kernel to verify that it was
loaded by the expected bootloader.

The value represents the ASCII string `IWBT`.

Since x86 uses little-endian byte ordering, the constant value is:

```c
#define BOOT_MAGIC 0x54425749
```

## Boot Info
`boot_info` is a data structure created by the bootloader and passed to the
kernel.

It contains hardware and environment information collected by the bootloader before transferring control to the kernel.

The current structure contains:
- RAM memory map

## Kernel Entry
The kernel entry point is the first code executed after stage 2 transfers
control to the kernel.

The entry code is responsible for adapting the bootloader calling convention
to the kernel C function calling convention.

Current implementation uses `cdecl` as calling convention so registers `EBX` and `EAX` are pushed on the stack.

The C kmain function has the following signature
```c
void kmain(uint32_t magic, boot_info_t *info);
```
