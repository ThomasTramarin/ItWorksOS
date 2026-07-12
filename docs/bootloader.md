# x86 Bootloader

A custom multi-stage x86 bootloader for **IwomhOS**.

## Boot Flow

### 1. BIOS
The boot process starts with the BIOS, running in Real Mode (16 bit). The BIOS executes POST (Power-On Self-Test) operations and then:  

1. Scans connected devices and finds a bootable medium (a storage device containing the magic boot signature `0xAA55` in its very first sector, at bytes 510 and 511).  
2. Copies this first sector (the Master Boot Record) from the device into physical RAM at address `0x0000:0x7C00`.  
3. Jumps to `0x0000:0x7C00` and the execution control is taken by the MBR Boot code (first 446 bytes).

### 2. MBR
The MBR has just to find an active partition, load the VBR (Volume Boot Sector) and jump to that sector. In particular:  
1. **Initialization**: it disables interrupts, resets main segments and sets the temp stack base pointer to `0x7C00` (growing downward).  
2. **Saving the Boot Drive**: when the BIOS loads the MBR, `dl` register contains the boot drive number. The MBR immediately saves this value.  
3. **Partition Table Scanning**: it scans the 4 partition entries to find an active/bootable partition (marked with the `0x80` byte).  
4. **Loading the VBR**:  
    - If an active partition is found, it extracts its starting LBA (VBR sector), it loads this sector at address `0x0000:0x0500`
    - Otherwise, the boot fails.  
5. **Jumping to the VBR code**:   
    - It passes some information to the VBR via CPU registers:  
        - `dl` contains the boot drive  
        - `ds:si` contains the memory address of the active partition's entry inside the MBR Partition Table  
    - It jumps to `0x0000:0x0500`, transferring the control to the VBR

## Memory Layout
- `0x0000:0x7C00`: MBR loading address
- `0x0000:0x0500`: VBR loading address
