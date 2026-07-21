[org 0x0700]
[bits 16]

%include "constants.asm"

stage2_entry:
    call stage2_init

halt:
    hlt
    jmp halt

;data

; this structure is passed to stage2 functions
stage2_context:
.boot_drive:
    db 0
.mbr_partition_entry:
    times 16 db 0
.bpb:
    times 25 db 0
.ebpb:
    times 54 db 0

; calculated values
.fat_start_lba:
    dd 0
.data_start_lba:
    dd 0
.bytes_per_cluster:
    dd 0

%include "memory.asm"
%include "video.asm"
%include "disk_read.asm"

; FUNC: initializes the stage2_context structure
; Input:
;   - DL = boot drive
;   - DS:SI = MBR boot partition entry
;   - ES:DI = VBR pointer
; Output: stage2_context populated
stage2_init:
    pusha

    ; store the boot drive ID
    mov [stage2_context.boot_drive], dl

    ; preserve VBR pointer (ES:DI) on the stack
    push es
    push di

    ; copy MBR partition entry (16 bytes from DS:SI)
    mov cx, 16
    mov di, stage2_context.mbr_partition_entry
    call memcpy

    ; restore VBR pointer from stack into DS:SI
    pop si      ; SI = VBR offset (0x0500)
    pop ds      ; DS = VBR segment (0x0000)

    ; copy BPB (25 bytes)
    ; skip the first 11 bytes of VBR (3 bytes jump instruction + 8 bytes OEM name)
    add si, 11
    mov cx, 25
    mov di, stage2_context.bpb
    call memcpy

    ; copy EBPB (54 bytes)
    ; advance SI by 25 bytes to point directly to the EBPB.
    add si, 25
    mov cx, 54
    mov di, stage2_context.ebpb
    call memcpy

    ; calculate bytes_per_cluster
    movzx eax, word [stage2_context.bpb + FAT32_BPB_BYTES_PER_SECTOR_OFF]
    movzx ebx, byte [stage2_context.bpb + FAT32_BPB_SECTORS_PER_CLUSTER_OFF]   
    imul eax, ebx   ; EAX (bytes_per_cluster) = bytes_per_sector * sectors_per_cluster
    mov [stage2_context.bytes_per_cluster], eax

    ; calculate fat_start_lba
    movzx ebx, word [stage2_context.bpb + FAT32_BPB_RESERVED_SECTORS_COUNT_OFF]
    imul eax, ebx ; EAX (fat_start_lba) = bytes_per_cluster * reserved_sectors_count 
    mov [stage2_context.fat_start_lba], eax

    ; calculate data_start_lba
    mov ebx, [stage2_context.ebpb + FAT32_EBPB_FAT_SIZE_32_OFF]
    movzx ecx, byte [stage2_context.bpb + FAT32_BPB_NUMBER_FATS_OFF]
    imul ebx, ecx ; EBX (tot_sectors_fats) = sectors_per_fat * num_of_fats

    add eax, ebx  ; EAX (data_start_lba) = fat_start_lba + tot_sectors_fats
    mov [stage2_context.data_start_lba], eax

    popa
    ret

