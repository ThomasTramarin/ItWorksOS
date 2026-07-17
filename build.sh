#!/bin/bash 

# TOOLCHAIN CONFIGURATION 
# Set the paths to locate the i686-elf cross-compiler
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

echo "--- [IwomhOS] Cross-compiler toolchain loaded ---"

make clean

make run
