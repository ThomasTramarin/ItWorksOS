# Boot Process

This document describes the complete boot sequence used by IWOS, from the BIOS
to the execution of the C kernel entry point.

## Overview

The boot process consists of multiple stages, each with a specific
responsibility.

The stages are:

| Stage | Execution Mode | Responsibility |
|---|---|---|
| BIOS | 16-bit Real Mode | Initialize the platform and load the MBR |
| MBR | 16-bit Real Mode | Locate the boot partition and load its VBR |
| VBR | 16-bit Real Mode | Locate and load Stage2 |
| Stage2 | 16-bit Real Mode → 32-bit Protected Mode | Load the kernel and prepare the execution environment |
| Kernel Entry | 32-bit Protected Mode | Perform architecture-specific kernel bootstrap |
| Kernel | 32-bit Protected Mode | Initialize the operating system |

The interface between Stage2 and the kernel entry point is defined by the
[Boot Protocol](./boot_protocol.md).

---

## Boot Flow

### BIOS → MBR

The system starts in 16-bit x86 Real Mode.

The BIOS performs the initial platform initialization and selects a boot
device. For a BIOS boot, the selected device must contain a valid boot
signature (`0x55 0xAA`) in its first sector.

The BIOS:

1. Loads the first sector of the selected boot device into memory at `0x7C00`.
2. Stores the boot drive number in `DL`.
3. Transfers execution to the loaded sector.

The MBR therefore receives:

| Register | Value |
|---|---|
| `DL` | BIOS boot drive number |

---

## MBR → VBR

The MBR (Master Boot Record) is the first sector of the selected boot
device.

It contains the MBR partition table and the boot code.

The MBR:

1. Sets up the stack and required registers.
2. Locates the bootable FAT32 LBA partition.
3. Loads the first sector of the selected partition, the Volume Boot Record
   (VBR), at `0x0500`.
4. Transfers execution to the VBR.

The MBR passes the following information to the VBR:

| Register | Value |
|---|---|
| `DL` | Boot drive number |
| `DS:SI` | Pointer to the selected partition table entry |

The MBR memory region (`0x7C00 - 0x7DFF`) must remain available until Stage2
has copied any information it requires from it.

---

## VBR → Stage2

The VBR is the first sector of the selected boot partition and contains the
FAT32 BPB/EBPB.

Because the VBR is constrained to a single 512-byte sector, it implements only
the minimum FAT32 functionality required to locate and load Stage2.

The VBR:

1. Initializes the FAT32 parameters required for filesystem access.
2. Locates `BOOT/STAGE2.BIN`.
3. Loads Stage2 into memory starting at `0x0700`.
4. Transfers execution to Stage2.

The VBR passes the following information to Stage2:

| Register | Value |
|---|---|
| `DL` | Boot drive number |
| `DS:SI` | Pointer to the selected partition table entry |
| `ES:DI` | Pointer to the loaded VBR |

### VBR Limitations

The limited space available in the VBR restricts the filesystem functionality
that can be implemented at this stage.

Current limitations include:

- Only standard 8.3 filenames are supported.
- Directory traversal is limited to the first cluster of the directory being
  searched.
- FAT validation is minimal.
- Directory entry validation is minimal.
- Errors are not reported to the user. If Stage2 cannot be loaded, execution
  is halted.

The VBR is therefore intentionally not a complete FAT32 implementation. Its
only purpose is to locate and load Stage2 and transfer execution to it.

---

## Stage2 → Kernel Entry

Stage2 is the main bootloader stage.

It initially executes in 16-bit Real Mode and is not constrained to a single
512-byte sector. This allows it to implement a more complete FAT32 loader and
perform the initialization required before entering the kernel.

Stage2 performs the following operations:

1. Preserves the information received from the VBR.
2. Initializes and validates the required FAT32 filesystem structures.
3. Locates `BOOT/KERNEL.BIN`.
4. Loads the kernel image at physical address `0x10000`.
5. Retrieves the physical memory map using BIOS `INT 0x15, E820`.
6. Constructs the `boot_info` structure.
7. Enables the A20 line.
8. Loads the Global Descriptor Table (GDT).
9. Switches the CPU from Real Mode to 32-bit Protected Mode.
10. Transfers execution to the x86 kernel entry point.

The CPU state and register-level interface established by Stage2 are defined
by the [Boot Protocol](./boot_protocol.md).

Stage2 may occupy physical memory that will later be reclaimed by the kernel.
Any information required by the kernel must therefore be preserved before
that memory is released.

---

## Kernel Entry → `kmain()`

The kernel entry point is the first kernel code executed after Stage2 transfers
control to the kernel.

The entry point is architecture-specific.

The x86 kernel entry point performs the minimal architecture-specific bootstrap
required before entering the C kernel. This includes establishing temporary
paging, switching execution to the higher-half kernel address space,
initializing the kernel `.bss` section, and setting up the kernel stack.

After the initial bootstrap is complete, the entry point transfers control to the C kernel main function (`kmain`).