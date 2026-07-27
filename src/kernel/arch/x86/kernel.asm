[BITS 32]
global _start
extern kmain

_start:

    push ebx    ; pointer to boot_info (cdecl)
    push eax    ; magic value
    call kmain

    jmp .halt

.halt:
    cli
    hlt
    jmp .halt

