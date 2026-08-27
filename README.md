# ItWorksOS (IWOS)

A bare-metal kernel for the x86 IA-32 architecture.

> Note: Originally named **ItWorksOnMyHP-OS** as a joke about the classic "it works on my machine", the project was renamed to **ItWorksOS** to reflect its primary goal: learning how operating systems work by building a small one that actually works.

> Note: this project is still under active development

## Overview

ItWorksOS is an educational operating system kernel developed to explore low-level system software and OS internals from scratch.

## Documentation
Technical documentation for internal subsystems is available in [`docs/`](docs/)

## Prerequisites
- `nasm`
- `i686-elf-gcc`: C cross-compiler
- `i686-elf-ld`: Linker
- `i686-elf-gdb`: GDB for the i686-elf target
- `make`
- `qemu-system-i386`: Emulator

## Build & Run
```bash

# Clone the repository
git clone https://github.com/ThomasTramarin/ItWorksOS.git
cd ItWorksOS

# Make the build script executable
chmod +x ./iwos

# Build the project (debug)
./iwos build

# Build and launch QEMU
./iwos run

# Clean build files
./iwos clean

# Build and launch qemu in debug mode
./iwos debug

# Connect GDB to the running QEMU instance (on a separate shell)
./iwos gdb

```

## Real Machine Testing
To test the operating system on real hardware, you can write the generated raw disk image (`bin/disk.img`) directly to a physical storage device, such as a USB flash drive.

> Warning: The `bin/disk.img` file contains the raw layout of a 64 MiB drive, formatted using MBR as partition scheme and FAT32 as the file system. Writing it to a USB drive will overwrite all existing data on that device.

### Writing to USB (Linux)
Identify your USB device (e.g. `/dev/sdX`) and write the image using `dd`:
```bash
# Replace /dev/sdX with your actual USB device
sudo dd if=bin/disk.img of=/dev/sdX bs=4M status=progress conv=fsync
```