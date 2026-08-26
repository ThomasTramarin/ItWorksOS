[bits 16]

; FUNC: read sectors from a disk and load data into RAM
;
; Input: 
;   EAX = LBA address (32 bit)
;   CX = number of sectors to read
;   DL = drive number
;   ES:SI = address to load data into
; Output:
;   CF = 0 (success), 1 (failure)
disk_read_lba:
    pusha

    mov di, cx  ; save the number of sectors to read

    sub sp, 16  ; allocate 16 bytes for storing the DAP
    mov bp, sp

    ; DAP (Disk Address Packet)
    mov byte [bp], 0x10     ; dap_size (16 byte)
    mov byte [bp+1], 0      ; dap_reserved
    mov [bp+2], cx          ; dap_sectors
    mov [bp+4], si          ; dap_offset (dest)
    mov [bp+6], es          ; dap_segment (dest)

    mov [ss:bp+8], eax      ; dap_lba_low
    mov dword [bp+12], 0x0  ; dap_lba_high (32 bits)

    push ds

    ; DS:SI = DAP in memory
    mov ax, ss
    mov ds, ax

    mov si, bp

    mov ah, 0x42    ; read LBA BIOS service
                    ; DL = drive number

    int 0x13

    pop ds

    jc .error

    ; partial read validation
    cmp di, [bp + 2]
    jne .error

.success:
    add sp, 16
    popa
    clc ; CF = 0
    ret

.error:
    add sp, 16
    popa
    stc
    ret
