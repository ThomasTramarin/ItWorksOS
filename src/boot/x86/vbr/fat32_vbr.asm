[org 0x0500]
[bits 16]

jmp_istr: 
    jmp boot_code
    nop

oem_name: db 'IWOMHOS '
%include "vbr/bpb.asm"
%include "vbr/ebpb.asm"


boot_code:

    mov [boot_drive], dl
    mov [mbr_boot_partition], si

    mov si, msg
    call video_print_string

.halt:
    hlt
    jmp .halt

; data
msg: db 'Test', 0
boot_drive: db 0
mbr_boot_partition: dw 0

%include "video.asm"

times 510 - ($-$$) db 0
dw 0xAA55