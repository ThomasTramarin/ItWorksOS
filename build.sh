#!/bin/bash 

# TOOLCHAIN CONFIGURATION 
# Set the paths to locate the i686-elf cross-compiler
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

echo "--- [IWOS] Cross-compiler toolchain loaded ---"

# CLI parsing
case "$1" in
    run)
        make clean
        make run
        ;;
    clean)
        make clean
        ;;
    build|"")
        make clean
        make
        ;;
    *)
        echo "Usage: $0 [build|run|clean]"
        exit 1
        ;;

esac
