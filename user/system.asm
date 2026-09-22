; System executable for IA-32
; This binary file defines "system" program, in the IWBF format
; The "system" process is the first process loaded by the kernel
; Currently, the IWBF header and segments are manually written in the binary

bits 32

; The executable is linked for virtual address 0x00400000
; NASM uses this value when resolving symbol addresses
org 0x00400000

; IWBF hdr
dd 'IWBF'
dw 1                ; EXEC
dw 1                ; I386
dd system_entry       ; entry  
dd 1                ; segment_count

; Segment Table (1 code segment)
dd 0x00400000           ; vaddr
dd segment_data - $$    ; file_off
dd segment_data_end - segment_data ; file_size
dd segment_data_end - segment_data ; mem_size
dd 0x5                  ; READ | EXEC


segment_data:

system_entry:
    jmp system_entry ; infinite loop

segment_data_end:

