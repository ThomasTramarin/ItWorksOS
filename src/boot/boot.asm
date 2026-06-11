[bits 16]
[org 0x7C00]


CODE_OFFSET equ 0x8             ; code segment sector in GDT
DATA_OFFSET equ 0x10            ; data segment sector in GDT

KERNEL_LOAD_SEG equ 0x1000      ; temporary Segment for ES (0x1000:0x0000)
KERNEL_START_ADDR equ 0x10000   ; final physical memory destination (kernel)

start:
    cli                 ; disable interrupts

    ; setup segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00      ; stack at bootloader top

    sti                 ; enable interrupts

; load the kernel
mov ax, KERNEL_LOAD_SEG

xor bx, bx
mov es, ax      ; ES:BX becomes the destination pointer (0x1000:0x0000 = 0x10000)

mov dl, 0x80    ; first drive ID
mov dh, 0x00    ; head = 0
mov cl, 0x02    ; sector = 2 (sector 1 is the bootloader)
mov ch, 0x00    ; cylinder 0
mov ah, 0x02    ; BIOS read sectors function
mov al, 32      ; read 32 sectors (~16 KB max kernel size)
int 0x13        

jc disk_read_error  ; if CF is set, something went wrong

; Load Protected Mode
load_PM:
    cli     ; disable interrupts

    ; enable fast A20 Gate via System Control Port A
    in al, 0x92
    or al, 2        ; enable A20 Gate (unlocks memory > 1MB)
    out 0x92, al

    ; Load Global Descriptor Table
    lgdt[gdt_descriptor]

    ; enter protected mode (32-bit)
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; far jump using the GDT code selector
    jmp CODE_OFFSET:PModeMain

disk_read_error:
    hlt
    jmp disk_read_error

; GDT implementation
gdt_start:
    ; NULL descriptor
    dd 0x0
    dd 0x0

    ; Code Segment Descriptor (offset: 0x8)
    dw 0xFFFF     ; limit: 4GB allocation size
    dw 0x0000     ; base: start at address 0x0
    db 0x00       ; base
    db 10011010b  ; access byte: present, ring 0, Executable, Code Seg
    db 11001111b  ; flags: 4KB granularity, 32-bit protected mode
    db 0x00       ; base

    ; Data Segment Descriptor (offset: 0x10)
    dw 0xFFFF     ; limit: 4GB allocation size
    dw 0x0000     ; base: start at address 0x0
    db 0x00       ; base
    db 10010010b  ; access byte: present, ring 0, writable, Data Seg
    db 11001111b  ; flags: 4KB granularity, 32-bit protected mode
    db 0x00       ; base

gdt_end:

; GDT Register Pointer structure needed by the 'lgdt' instruction
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]
PModeMain:
    ; update all segment registers to use the new Data Segment (0x10)
    mov ax, DATA_OFFSET
    mov ds, ax 
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov gs, ax

    ; stack 0x90000
    mov ebp, 0x90000
    mov esp, ebp

    ; far jump into the compiled kernel at 0x10000 
    jmp CODE_OFFSET:KERNEL_START_ADDR

times 446 - ($ - $$) db 0

; Partition Table
; temporary fake partition to allow running on real hardware
db 0x80                 ; bootable
db 0x01, 0x01, 0x00     ; CHS start address
db 0x0C                 ; partition type: FAT32 LBA
db 0xFE, 0xFFFF, 0xFFFF ; CHS end address   
dd 0x00000001           ; hidden sectors before partition (LBA 1)
dd 0x00010000           ; total sectors in partition (32)

times 48 db 0   ; fill remaining partition table empty slots

dw 0xAA55   ; MBR signature