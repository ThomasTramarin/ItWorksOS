; System executable for IA-32
; This binary file defines "system" program, in the IWBF format
; The "system" process is the first process loaded by the kernel
; Currently, the IWBF header and segments are manually written in the binary

bits 32

; Virtual addresses
%define CODE_VADDR 0x00400000
%define DATA_VADDR 0x00401000

; The executable is linked for virtual address 0x00400000
; NASM uses this value when resolving symbol addresses
org 0x00400000

; IWBF hdr
dd 'IWBF'
dw 1                ; EXEC
dw 1                ; I386
dd 0x00400000       ; entry  
dd 2                ; segment_count

; Segment Table

; code segment
dd CODE_VADDR           ; vaddr
dd segment_code - $$    ; file_off
dd segment_code_end - segment_code ; file_size
dd 4096 ; mem_size
dd 0x5                  ; READ | EXEC

; data segment
dd DATA_VADDR           ; vaddr
dd segment_data - $$    ; file_off
dd segment_data_end - segment_data ; file_size
dd 4096 ; mem_size
dd 0x3                  ; READ | WRITE

segment_code:

system_start:

    mov eax, 0                                  ; sys_write
    mov ebx, DATA_VADDR + (msg - segment_data)  ; buf
    mov ecx, msg_size                           ; len
    int 0x80


.lop:
    jmp .lop ; infinite loop

segment_code_end:

segment_data:

msg: db 'Hello from system process!', 10
msg_size: equ $ - msg

segment_data_end:

