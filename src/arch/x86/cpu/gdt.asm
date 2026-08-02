
global x86_gdt_flush
section .text

; FUNC: loads the GDT and sets segments to the new 
;       kernel code/data segments
; Parameters:   struct x86_gdt_descriptor * (cdecl)
; Return:       void
x86_gdt_flush:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8] ; GDT descriptor pointer

    lgdt [eax]

    ; Data segment
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Code segment (flush CS via far jump)
    jmp 0x08:.flush_cs

.flush_cs:

    pop ebp
    ret
