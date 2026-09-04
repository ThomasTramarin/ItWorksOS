[bits 32]

extern x86_isr_handler

; If the interrupt happens at the same privilege level:
;
; CPU automatically pushes:
;   old EFLAGS
;   old CS
;   old EIP
;
; The current stack is used.
;
; If the interrupt happens from a lower privilege level:
;
; CPU first switches to the new privilege stack defined by the TSS.
;
; Then CPU pushes the old user stack information:
;   old SS
;   old ESP
;
; Then CPU pushes:
;   old EFLAGS
;   old CS
;   old EIP

%macro ISR_NOERROR 1

global x86_isr_stub%1
x86_isr_stub%1:
    push 0  ; dummy err code
    push %1 ; push interrupt number
    jmp x86_isr_common

%endmacro

%macro ISR_ERROR 1

global x86_isr_stub%1
x86_isr_stub%1:
            ; CPU pushes an error code to the stack
    push %1 ; push interrupt number
    jmp x86_isr_common

%endmacro

section .text
ISR_NOERROR 0
ISR_NOERROR 1
ISR_NOERROR 2
ISR_NOERROR 3
ISR_NOERROR 4
ISR_NOERROR 5
ISR_NOERROR 6
ISR_NOERROR 7
ISR_ERROR 8
ISR_NOERROR 9
ISR_ERROR 10
ISR_ERROR 11
ISR_ERROR 12
ISR_ERROR 13
ISR_ERROR 14
ISR_NOERROR 15
ISR_NOERROR 16
ISR_ERROR 17
ISR_NOERROR 18
ISR_NOERROR 19
ISR_NOERROR 20
ISR_ERROR 21
ISR_NOERROR 22
ISR_NOERROR 23
ISR_NOERROR 24
ISR_NOERROR 25
ISR_NOERROR 26
ISR_NOERROR 27
ISR_NOERROR 28
ISR_ERROR 29
ISR_ERROR 30
ISR_NOERROR 31

%assign i 32

%rep 224
    ISR_NOERROR i

%assign i i+1

%endrep

x86_isr_common:
    pushad   ; push general-purpose registers
            ; order: eax, ecx, edx, ebx, esp, ebp, esi, edi

    ; save segment registers
    push ds
    push es
    push fs
    push gs

    ; load kernel data segments
    mov ax, 0x10 
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax


    ; pass a pointer to the interrupt stack frame (cdecl)
    push esp

    call x86_isr_handler

    add esp, 4  ; remove function argument

    ; restore segment registers
    pop gs
    pop fs
    pop es
    pop ds

    popad ; restore general-purpose registers

    add esp, 8 ; remove err code and interrupt number

    iretd


%macro ISR_TABLE_ENTRY 1
    dd x86_isr_stub%1   
%endmacro

; table of function pointers to isr stubs
global x86_isr_stub_table

section .init.data
x86_isr_stub_table:
%assign i 0
%rep 256
    ISR_TABLE_ENTRY i
%assign i i+1
%endrep