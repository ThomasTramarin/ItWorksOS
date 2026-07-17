# mkimage

The `mkimage` utility acts as a FAT32 image formatter and file injector.

## What it does
`mkimage` takes your raw bootloader binaries and a list of host files, producing a ready-to-boot, partitioned disk image. Specifically, the tool performs the following operations:

1. **Image Initialization:** Creates a blank, zero-padded disk image of the required target size.
2. **Bootloader Embedding:** Writes the Master Boot Record (MBR) to the very first sector (LBA 0) and injects the Volume Boot Record (VBR) at the start of the primary partition.
3. **Filesystem Construction:** Builds a valid FAT32 filesystem layout from scratch, generating the File Allocation Tables (FAT) and establishing the root directory cluster.
4. **Automated File Injection:** Parses a configuration mapping file (`disk.files`), creates any necessary subdirectories inside the virtual disk, and writes your host files (such as `kernel.bin` or configuration files) directly into the allocated FAT32 data clusters.

## CLI Usage
The tool is executed via the command line, requiring four positional arguments:

```sh
mkimage <mbr.bin> <vbr.bin> <output.img> <disk.files>
```
- `<mbr.bin>`: The compiled 512-byte Master Boot Record binary.

- `<vbr.bin>`: The compiled FAT32 Volume Boot Record binary (contains the BPB).

- `<output.img>`: The path for the generated output disk image.

- `<disk.files>`: A text file containing line-by-line mappings of host files to destination paths inside the image.

## Syntax for `disk.files`
The mapping file supports two types of operations:
1. **File Injection (with automatic parent directory creation)**:
```
hello.txt:boot/hello.txt
```
Copies the file `hello.txt` from the host machine to `/boot/hello.txt` inside the image. If `/boot/` does not exist, it is automatically created.

2. **Empty Directory Creation**:
```
:boot/
```
By leaving the host path empty and adding a trailing slash `/` to the destination, the tool creates empty directories inside the image without searching for files on your host machine.

## Current Limitations
- **Partitioning**: This tool is designed to format exactly one FAT32 partition. The image builder reads partition parameters (such as the starting LBA) strictly from the first entry of the MBR partition table. Consequently, the FAT32 partition must be defined as the first and only bootable entry in your MBR.
- **Directory Capacity**: Each directory entry within the FAT32 filesystem occupies exactly 32 bytes. With a cluster size of 512 bytes (1 sector), a single directory cluster can hold a maximum of 16 directory entries. That's because the current version allocates only one cluster for a directory entry, but it's acceptable for now.  
    - Note: these 16 entries include the mandatory `.` (current directory) and `..` (parent directory) entries.  
- **Fsinfo**: The current image generator doesn't yet write a valid fsinfo sector.
- **LFN Support**: This tool implements basic 8.3 filename support.
- **No Direct Host Directory Import**: The tool cannot recursively import an entire directory (e.g. mapping `host_folder/:image_folder/` will fail). Every file must be explicitly mapped line-by-line inside `disk.files`, or created as an empty directory.