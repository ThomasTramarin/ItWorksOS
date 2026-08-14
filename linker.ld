ENTRY(_start)
OUTPUT_FORMAT(binary)
SECTIONS {
    . = 0xC0010000;

    kernel_start = .;

    .text : AT(ADDR(.text) - 0xC0000000)
    {
        *kernel.asm.o(.text)
        *(.text*)
    }

    .rodata : AT(ADDR(.rodata) - 0xC0000000)
    {
        *(.rodata*)
    }

    .data : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data*)
    }

    .bss ALIGN(4096) : AT(ADDR(.bss) - 0xC0000000)
    {
        bss_start = .;
        *(COMMON)
        *(.bss*)
        bss_end = .;
    }

    kernel_end = .;
}