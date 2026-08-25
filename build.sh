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
        make BUILD=debug run
        ;;
    build|"")
        make clean
        make BUILD=debug
        ;;
    release)
        make clean
        make BUILD=release
        ;;
    clean)
        make BUILD=debug clean
        ;;
    *)
        echo "Usage: $0 [build|release|run|clean]"
        exit 1
        ;;

esac
