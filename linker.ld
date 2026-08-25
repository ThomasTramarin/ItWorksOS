ENTRY(_start)

SECTIONS
{
    . = 0xC0010000;

    kernel_start = .;

    .text ALIGN(4096) : AT(ADDR(.text) - 0xC0000000)
    {
        *kernel.asm.o(.text)
        *(.text*)
    }

    .rodata ALIGN(4096) : AT(ADDR(.rodata) - 0xC0000000)
    {
        *(.rodata*)
    }

    .data ALIGN(4096) : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data*)
    }

    .bootstrap_bss ALIGN(4096) : AT(ADDR(.bootstrap_bss) - 0xC0000000)
    {
        bootstrap_bss_start = .;
        *(.bootstrap_bss*)
        bootstrap_bss_end = .;
    }

    .bss ALIGN(4096) : AT(ADDR(.bss) - 0xC0000000)
    {
        bss_start = .;
        *(COMMON)
        *(.bss*)
        bss_end = .;
    }

    . = ALIGN(4096);
    kernel_end = .;
}