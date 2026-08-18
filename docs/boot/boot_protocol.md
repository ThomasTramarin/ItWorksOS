# Boot Protocol

The boot protocol defines the interface between the bootloader (Stage2) and
the kernel entry point.

It specifies the CPU state, register values, and boot information available
to the kernel when Stage2 transfers control to it.

## CPU State

At the moment of transferring execution to the kernel entry point, Stage2
leaves the CPU in the following state:

- **Execution Mode**: 32-bit Protected Mode
- **Interrupts**: disabled
- **Paging**: disabled
- **A20 Line**: enabled
- **GDT**: loaded with flat-model code and data descriptors
- **Code Segment**: points to a flat-model GDT code descriptor
- **Data Segments**: point to a flat-model GDT data descriptor
- **Stack Pointer**: points to a valid stack region owned by Stage2

The kernel entry point must not rely on the Stage2 stack after its initial
bootstrap sequence.

The kernel entry point is responsible for establishing its own stack before
entering the C kernel.

## Register State

Stage2 provides the following values to the kernel entry point:

| Register | Value |
|---|---|
| `EAX` | `BOOT_MAGIC` |
| `EBX` | Physical address of `boot_info` |
| `ESP` | Valid Stage2 stack |

All other general-purpose registers are unspecified and must not be relied
upon by the kernel entry point.

## Boot Magic

`BOOT_MAGIC` is a 32-bit constant used by the kernel to verify that it was
entered through the expected IWOS boot protocol.

The value represents the ASCII string `IWBT`.

Because x86 uses little-endian byte ordering, the constant is:

```c
#define BOOT_MAGIC 0x54425749
```

The kernel entry point must preserve the value provided in `EAX` before using
the register for other purposes.

## Boot Info
`boot_info` is a structure created by Stage2 and passed to the kernel.

It contains information collected by the bootloader that is required by the
kernel during early initialization.

The current structure contains:

Physical memory map obtained through BIOS E820.

Stage2 provides the physical address of this structure in `EBX`.

The kernel is responsible for preserving any required boot information before
the physical memory occupied by Stage2 is reclaimed.

## Kernel Entry
The kernel is linked to execute in the higher half of the virtual address
space, while Stage2 initially loads the kernel into physical memory.

Therefore, the kernel entry point must establish a minimal virtual memory mappings through paging required to execute the kernel using its virtual addresses.

The bootstrap page tables provide:
- an identity mapping of the first 4 MiB of physical memory
- a higher-half mapping from virtual address `0xC0000000` to physical address `0x00000000`

These mappings are temporary and the definitive virtual address space is initialized later by the kernel's VMM (Virtual Memory Manager).

## C Kernel Entry
After setting up the required kernel execution environment, the x86 kernel entry point invokes the C kernel main function:
```c
void kmain(uint32_t magic, paddr_t boot_info);
```

| Argument    | Value                                         |
| ----------- | --------------------------------------------- |
| `magic`     | `BOOT_MAGIC`                                  |
| `boot_info` | Physical address of the `boot_info` structure |

