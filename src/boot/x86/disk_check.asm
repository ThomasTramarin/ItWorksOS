[bits 16]

; Note: this function should be called by the MBR code before loading VBR
; so if LBA extensions are not supported the system doesn't boot and
; return an error.
; On VBR, bytes are crucial and, since if we are executing the VBR boot code
; it means that there are BIOS extensions, the VBR simply include only
; disk_read.asm

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