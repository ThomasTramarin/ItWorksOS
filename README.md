# IwomhOS (ItWorksOnMyHP Operating System)

A bare-metal kernel for the x86 architecture 

> Note: this project is still under active development

## Overview

This is a learning project focused on understanding low-level system management.

## Features
- **Architecture**: x86 32-bit (Protected Mode)
- **Bootloader**: Minimal bootloader on MBR (Master Boot Record)
- **GDT**: Basic Global Description Table implementation
- **IDT**: Basic IDT implementation and centralized ISR dispatcher
- **Exceptions**: Dedicated CPU exception handlers (0-31) with registers dump and Kernel Panic screen  
- **PIC**: Programmable Interrupt Controller (8259A) driver implementation (initialization, IRQs remap, spurious IRQ handling)
- **VGA**: Vga text mode driver
- **Timer**: Timer interrupt driver (Programmable Interval Timer)
- **Kernel Library**:   
    - `conv`: Conversion module (Integer to ASCII function)
    - `va`: Variadic function arguments module  
    - `cui`: Character User Interface kernel module (abstraction layer over the VGA driver)
    - `time`: Time module providing system tick counter and sleep functionality
    - `types`: Basic data type definitions

## Prerequisites
- `nasm`
- `i686-elf-gcc`: C cross-compiler
- `i686-elf-ld`: Linker
- `make`
- `qemu-system-i386`: Emulator

## Build & Run
```bash

# clone the repository
git clone https://github.com/ThomasTramarin/ItWorksOnMyHP-OS.git
cd ItWorksOnMyHP-OS

# Make the script executable
chmod +x ./build.sh

# Build the project
./build.sh

# run in QEMU
qemu-system-i386 -hda bin/os.bin
```