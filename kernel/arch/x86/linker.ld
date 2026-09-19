ENTRY(_start)

SECTIONS
{
    . = 0xC0010000;

    __kernel_start = .;

    .init.text ALIGN(4096) : AT(ADDR(.init.text) - 0xC0000000)
    {
        __init_text_start = .;
        KEEP(*kernel.asm.o(.text))
        *(.init.text*)

        . = ALIGN(4096);
        __init_text_end = .;
    }

    .text ALIGN(4096) : AT(ADDR(.text) - 0xC0000000)
    {
        __text_start = .;
        *(.text*)

        . = ALIGN(4096);
        __text_end = .;
    }

    .data ALIGN(4096) : AT(ADDR(.data) - 0xC0000000)
    {
        __data_start = .;
        *(.data*)

        . = ALIGN(4096);
        __data_end = .;
    }

    .rodata ALIGN(4096) : AT(ADDR(.rodata) - 0xC0000000)
    {
        __rodata_start = .;
        *(.rodata*)

        . = ALIGN(4096);
        __rodata_end = .;
    }

    .initcalls ALIGN(4096) : AT(ADDR(.initcalls) - 0xC0000000)
    {
        __initcall_devdrv_start = .;
        KEEP(*(.initcall.devdrv*))
        __initcall_devdrv_end = .;
    }

    .init.bss ALIGN(4096) : AT(ADDR(.init.bss) - 0xC0000000)
    {
        __init_bss_start = .;
        *(.init.bss*)

        . = ALIGN(4096);
        __init_bss_end = .;
    }

    .bss ALIGN(4096) : AT(ADDR(.bss) - 0xC0000000)
    {
        __bss_start = .;
        *(COMMON)
        *(.bss*)

        . = ALIGN(4096);
        __bss_end = .;
    }

    __kernel_end = .;
}