[bits 16]


; FUNC: checks if the BIOS supports LBA addressing instead of CHS
;
; Input: DL = drive number
; Output: CF = 0 if supported, 1 if not supported
disk_check_lba_extensions:
    pusha

    mov ah, 0x41    ; check presence of BIOS extensions
    mov bx, 0x55AA  ; magic number
                    ; DL contains the drive number
    int 0x13

    ; supported: 
    ;   CF = 0, BX with bytes swapped (becomes 0xAA55)
    ;   AH, DH contains some version information
    ;   CX contains a bitmask of supported commands in AH
    ; not supported: 
    ;   CF = 1, BX not altered

    jc .not_supported
    cmp bx, 0xAA55
    jne .not_supported

    ; the LSB of CX should be set (extended read/write support)
    test cx, 1 ; CX and 00000000 00000001 
    jz .not_supported

.supported:
    popa
    clc     ; force CF = 0
    ret

.not_supported:
    popa
    stc     ; force CF = 1
    ret


; FUNC: read sectors from a disk and load data into RAM
;
; Input: 
;   AX = LBA address (lowest 16 bits)
;   BX = LBA address (highest 16 bits)
;   CX = number of sectors to read
;   DL = drive number
;   ES:SI = address to load data into
; Output:
;   CF = 0 (success), 1 (failure)
disk_read_lba:
    pusha

    call disk_check_lba_extensions
    jc .error_no_stack

    mov di, cx  ; save the number of sectors to read

    sub sp, 16  ; allocate 16 bytes for storing the DAP
    mov bp, sp

    ; DAP (Disk Address Packet)
    mov byte [bp], 0x10     ; dap_size (16 byte)
    mov byte [bp+1], 0      ; dap_reserved
    mov [bp+2], cx          ; dap_sectors
    mov [bp+4], si          ; dap_offset (dest)
    mov [bp+6], es          ; dap_segment (dest)
    mov [bp+8], ax          ; dap_lba_low (first 16 bits)
    mov word [bp+10], bx    ; dap_lba_low (second 16 bits)
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

    jc .error_with_stack

    ; partial read validation
    cmp di, [bp + 2]
    jne .error_with_stack

.success:
    add sp, 16
    popa
    clc ; CF = 0
    ret

.error_with_stack:
    add sp, 16
.error_no_stack:
    popa
    stc
    ret
