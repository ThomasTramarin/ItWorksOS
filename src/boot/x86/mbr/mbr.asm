[org 0x7c00]
[bits 16]

mbr_stage:

    cli     ; disable interrupts

    ; reset all segments
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov bp, 0x7c00

    sti     ; enable interrupts

    mov [boot_drive], dl    ; save the boot drive number

    mov si, mbr_partition_table
    call find_active_partition

    cmp ax, 64
    je .err_no_partition

    ; valid bootable partition found

    mov si, mbr_partition_table

    add si, ax  ; DS:SI points to the first byte of the selected 16-byte MBR entry

    push ds
    push si

    ; LBA start field is at offset 8 of the entry
    mov eax, [ds:si + 8] 

    mov si, 0x500   ; VBR will be loaded at 0x0000:0x0500
    mov cx, 1
    mov dl, [boot_drive]

    call disk_check_lba_extensions
    jc .err_lba_ext

    call disk_read_lba

    jc .err_vbr_load_error


    ; the VBR receives:
                ; the boot drive in DL
    pop si      ; DS:SI containing the memory address of the boot partition
    pop ds

    jmp 0x0000:0x0500   ; jump to VBR

.err_no_partition:
    mov si, msg_no_active_partition
    call video_print_string
    jmp .halt

.err_vbr_load_error:
    mov si, msg_vbr_load_error
    call video_print_string
    jmp .halt

.err_lba_ext:
    mov si, msg_no_lba_extensions
    call video_print_string
    jmp .halt

.halt:
    hlt
    jmp .halt

%include "mbr/partition.asm"
%include "video.asm"
%include "disk_read.asm"
%include "disk_check.asm"

; data
boot_drive: db 0
msg_no_active_partition: db 'Boot failed (MBR): no active partition found', 0xD, 0xA, 0
msg_vbr_load_error: db 'Boot failed (MBR): failed to load VBR sector', 0xD, 0xA, 0
msg_no_lba_extensions db 'Boot Failed (MBR): the BIOS does not support BIOS extensions', 0xD, 0xA, 0

times 446 - ($-$$) db 0 

mbr_partition_table:

%include "mbr/partition_table.asm"

dw 0xAA55   ; signature