# x86 Bootloader

A custom multi-stage x86 bootloader for **IwomhOS**.

## Overview
The boot chain consists of three stages:
```
BIOS -> Bootloader (MBR -> VBR -> Stage2) -> Kernel
```

Each stage has a specific responsibility:
- **MBR**: partition selection and VBR loading (512 bytes)
- **VBR**: FAT32 minimal (and restrictive) loader, finds and loads stage2 (512 bytes)
- **Stage2**: full boot environment and kernel loading (dynamic size)

## Boot Flow

### 1. BIOS
The system starts in 16-bit Real Mode.

Responsibilities:
- POST execution
- hardware initialization
- selecting boot device
- loading first sector into memory

The BIOS:
1. Searches for a bootable device
2. Checks the boot signature (`0x55 0xAA`)
3. Loads the first sector (at `0x0000:0x7C00`)
4. Sets `DL` = boot drive number
5. Transfers excevution to the `MBR` boot code

### 2. MBR
The MBR is the first sector (512 bytes) of the BIOS-selected boot drive

The MBR:
1. finds active partition
2. loads its VBR (at `0x0000:0x0500`)
3. transfers execution

### MBR ABI
Input from BIOS:
- `DL` = boot drive (provided by BIOS)

Output to VBR
- `DL` = boot drive
- `DS:SI` = pointer to active partition entry

## 3. VBR
The VBR is the first sector of the FAT partition marked as bootable and it is loaded by the MBR at `0x0000:0x0500`.

The VBR contains:
- FAT32 BPB and EBPB (information about the FAT partition)
- minimal FAT loader

The VBR:
1. calculates FAT Area and Data Area first sectors
3. finds: `/BOOT/STAGE2.BIN`
4. loads stage2 into memory
5. Jumps to stage2

### VBR ABI
Input from MBR:
- `DL` = boot drive
- `DS:SI` = pointer to active partition entry

Output to Stage2:
- `DL` = boot drive
- `DS:SI` = pointer to active partition entry
- `DS:DI` = pointer to the VBR previously loaded

### Notes about FAT loader implementation
The space available for the code is not too much (420 bytes), so the FAT loader at this stage is really basic.

So at this stage, there are some limitations:
1. When reading the content of a directory to find a file/subdirectory entry inside it, only the first cluster is loaded. Thus, the metadata of the entry to find, must be stored on the first cluster of the parent directory. So, if the cluster size is 512 bytes, and one entry is 32 bytes, this implementation will search the child within the first 16 entries.
2. No LFN support. The implementation will find only standard 8.3 entries.
3. Since `stage2.bin` is loaded at `0x0000:0x0700`, and the safe zone is up to `0x0000:0x7DFF` (because after that address there is the MBR sector), the usable memory for the bootloader is ~29.25KiB, which is not really a problem.
4. The code does not validate FAT (e.g. bad clusters, reserved clusters), it checks only free "normal" and free states.
5. The code does not validate directory entries (e.g. volume label, the entry type) but only looks at the 8.3 name and first cluster field.
6. Errors are not noticed, if something goes wrong, the stage2 will not be loaded and the CPU will be halted until the user switch off the computer.  

In conclusion, the VBR implements only the minimum functionality required to bootstrap the next stage.

Its goal is not to provide a complete FAT32 implementation, but only to locate and load `STAGE2.BIN` into memory and transfer execution.

More advanced features are delegated to Stage2, where the code size is no longer limited by the 512-byte boot sector constraint.

## Memory Layout

### MBR
The MBR is loaded at `0x7C00 - 0x7DFF` (512 bytes) by the BIOS.
It contains the partition table and transfers execution to the VBR.

The MBR memory area must be preserved until Stage2 has copied the required partition information.

### VBR
The VBR is loaded at `0x0500 - 0x06FF` (512 bytes) by the MBR.

During execution, the VBR uses these memory areas:

- `0x0500 - 0x06FF`: VBR code, BPB/EBPB, and internal variables.
  Must not be overwritten until Stage2 has copied the required filesystem information.

- `0x0700 - ...`: temporary buffer used to load FAT clusters and the Stage2 bootloader itself.
  After Stage2 takes control, this memory becomes owned by Stage2.

- `0x7E00 - 0x7FFF`: temporary buffer used to load FAT sectors for FAT table calculations.
  Can be reused after the VBR no longer needs FAT access.