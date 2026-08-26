[BITS 32]
global _start
extern kmain

extern bss_start
extern bss_end

; X86 Kernel entry point
;
; The bootloader enters the kernel in 32-bit protected mode and provides:
;   EAX = boot magic value
;   EBX = physical address of boot_info
;
; This entry point performs the minimu, CPU and memory setup required before 
; transferring control to the C kernel
;
; Bootstrap paging uses temporary 4MiB pages (PSE enabled) to map the kernel
; code at higher-half. The definitive paging is initialized later by the kernel
; using normal 4KiB pages
;
; Bootstrap Virtual Memory Layout:
;   0x00000000 -> 0x00000000    Identity mapping, first 4MiB
;   0xC0000000 -> 0x00000000    Higher-half mapping, first 4MiB


HIGH_MEM_ADDR   equ 0xc0000000
PAGE_4MB_FLAGS  equ 0x00000083 ; Present + Writable + PSE (4MiB pages)
PD_INDEX_KERNEL equ 768       ; 0xC0000000 >> 22
STACK_SIZE      equ 16384      ; 16 KiB


; Bootstrap BSS section
; This section is marked as nobits, so it occupies no bytes in the final kernel image
; The section stores the bootstrap page directory.
;
; The reason for not using the .bss section is because, once the paging is enabled,
; The .bss section will be cleared as required by C.
; If the page directory were stored in .bss, clearing the section would also
; invalidate the active page directory pointed by CR3, causing a page fault

section .bootstrap_bss nobits
align 4096      ; alignment required by CR3
boot_page_directory:
    resb 4096

; Kernel stack
section .bss
align 16
kernel_stack_bottom:
    resb STACK_SIZE
kernel_stack_top:

section .text
_start:
    mov esi, eax ; magic value
    mov edi, ebx ; pointer to boot_info (physical address)

    ; Check PSE support (CPUID.01H.EDX bit 3)
    mov eax, 1
    cpuid
    test edx, (1 << 3)
    jz halt

    ; Calculate the physical address of the boot_page_directory (for cr3 register)
    mov edx, (boot_page_directory - HIGH_MEM_ADDR)

    ; Map 0x00 -> 0x00 (identity map 0-4 MiB)
    mov dword [edx], PAGE_4MB_FLAGS

    ; Map 0xc0000000 -> 0x00 (higher half 3 GiB -> 0-4 MiB)
    mov dword [edx + PD_INDEX_KERNEL * 4], PAGE_4MB_FLAGS

    ; Enable PSE (Page Size Extension 4 MiB)
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax

    ; Load page directory
    mov cr3, edx

    ; Enable paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; Jump to higher half kernel code
    lea eax, [higher_half]
    jmp eax


higher_half:

    mov edx, edi

    ; Zero .bss section
    xor eax, eax
    mov edi, bss_start
    mov ecx, bss_end
    sub ecx, edi
    rep stosb

    mov edi, edx

    ; Initialize the kernel stack
    mov esp, kernel_stack_top

    ; Cdecl calling convention: kmain(magic, boot_info)
    push edi    ; boot_info
    push esi    ; magic

    call kmain

halt:
    cli
    hlt
    jmp halt
