[org 0x0700]
[bits 16]

; Stage2 Bootloader Code
; Responsibilities:
;   - Save previous boot stages information
;   - Validate the format of MBR partition and VBR parameters 

%include "constants.asm"

; --- Entry Point ---
stage2_entry:
    call stage2_init

    call stage2_validate

halt:
    hlt
    jmp halt


; --- Functions ---

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
    mov eax, [stage2_context.bpb + FAT32_BPB_HIDDEN_SECTORS_OFF]
    movzx ebx, word [stage2_context.bpb + FAT32_BPB_RESERVED_SECTORS_COUNT_OFF]
    add eax, ebx ; EAX (fat_start_lba) = hidden_sectors + reserved_sectors_count 
    mov [stage2_context.fat_start_lba], eax

    ; calculate data_start_lba
    mov ebx, [stage2_context.ebpb + FAT32_EBPB_FAT_SIZE_32_OFF]
    movzx ecx, byte [stage2_context.bpb + FAT32_BPB_NUMBER_FATS_OFF]
    imul ebx, ecx ; EBX (tot_sectors_fats) = sectors_per_fat * num_of_fats

    add eax, ebx  ; EAX (data_start_lba) = fat_start_lba + tot_sectors_fats
    mov [stage2_context.data_start_lba], eax

    popa
    ret

; FUNC: prints the error string to the terminal and halts the CPU
; Input:
;   - SI: the error string to print
; Output: None (this function never retruns, it enters an infinte loop)
stage2_error:
    push si

    mov si, msg_err_prefix
    call video_print_string

    pop si
    call video_print_string

.halt:
    cli
    hlt
    jmp .halt


; FUNC: validates MBR and FAT32 formats
; Input: None
; Output: None (automatically calls stage2_error function if error, otherwise, it returns)
stage2_validate:
    pusha

    ; --- MBR partition validation ---

    ; Check MBR partition bootable flag (should be 0x80 on active partition)
    mov al, [stage2_context.mbr_partition_entry + MBR_PARTITION_BOOT_INDICATOR_OFF]
    cmp al, 0x80
    jne .err_boot_indicator

    ; Check MBR partition type (should be FAT32 LBA -> 0x0C)
    mov al, [stage2_context.mbr_partition_entry + MBR_PARTITION_TYPE_OFF]
    cmp al, 0x0C
    jne .err_not_fat32_lba

    ; bpb_hidden_sectors should be equal to mbr_start_lba
    mov eax, [stage2_context.mbr_partition_entry + MBR_PARTITION_LBA_START_OFF]
    mov ebx, [stage2_context.bpb + FAT32_BPB_HIDDEN_SECTORS_OFF]
    cmp eax, ebx    ; lba_start == hidden_sectors ?
    jne .err_hidden_sectors_mismatch

    ; mbr_start_lba (and hidden_sectors) should not be 0
    cmp eax, 0
    je .err_invalid_hidden_sectors

    ; --- BPB validation ---

    ; bytes_per_sectors should be 512 (BIOS compatibility and semplicity)
    mov ax, [stage2_context.bpb + FAT32_BPB_BYTES_PER_SECTOR_OFF]
    cmp ax, 512
    jne .err_bytes_per_sector

    ; sectors_per_cluster should not be 0
    mov al, [stage2_context.bpb + FAT32_BPB_SECTORS_PER_CLUSTER_OFF]
    cmp al, 0
    je .err_sectors_per_cluster

    ; number_fats should not be 0
    mov al, [stage2_context.bpb + FAT32_BPB_NUMBER_FATS_OFF]
    cmp al, 0
    je .err_number_fats

    ; root_cluster should be >= 2
    mov eax, [stage2_context.ebpb + FAT32_EBPB_ROOT_CLUSTER_OFF]
    cmp eax, 2
    jb .err_root_cluster           

    ; total_sectors_16 should be 0 (on FAT32)
    mov ax, [stage2_context.bpb + FAT32_BPB_TOTAL_SECTORS_16_OFF]
    cmp ax, 0
    jne .err_total_sectors_16

    ; total_sectors_32 should be > 0 (on FAT32)
    mov eax, [stage2_context.bpb + FAT32_BPB_TOTAL_SECTORS_32_OFF]
    cmp eax, 0
    je .err_total_sectors_32

    ; fat_size_32 should be > 0
    mov eax, [stage2_context.ebpb + FAT32_EBPB_FAT_SIZE_32_OFF]
    cmp eax, 0
    je .err_fat_size_32

    popa
    ret ; return on success


.err_boot_indicator:
    mov si, msg_err_boot_indicator
    jmp stage2_error

.err_not_fat32_lba:
    mov si, msg_err_not_fat32_lba
    jmp stage2_error

.err_hidden_sectors_mismatch:
    mov si, msg_err_hidden_sectors_mismatch
    jmp stage2_error

.err_invalid_hidden_sectors:
    mov si, msg_err_invalid_hidden_sectors
    jmp stage2_error

.err_bytes_per_sector:
    mov si, msg_err_bytes_per_sector
    jmp stage2_error

.err_sectors_per_cluster:
    mov si, msg_err_sectors_per_cluster
    jmp stage2_error

.err_number_fats:
    mov si, msg_err_number_fats
    jmp stage2_error

.err_root_cluster:
    mov si, msg_err_root_cluster
    jmp stage2_error

.err_total_sectors_16:
    mov si, msg_err_total_sectors_16
    jmp stage2_error

.err_total_sectors_32:
    mov si, msg_err_total_sectors_32
    jmp stage2_error

.err_fat_size_32:
    mov si, msg_err_fat_size_32 
    jmp stage2_error

; FUNC: converts a FAT32 cluster number to its corresponding LBA address
; Input:
;   - EAX = Cluster number (must be >= 2)
; Output:
;   - EAX = Starting LBA address of the cluster
fat32_cluster_to_lba:
    push ebx

    sub eax, 2
    movzx ebx, byte [stage2_context.bpb + FAT32_BPB_SECTORS_PER_CLUSTER_OFF]
    imul eax, ebx
    add eax, [stage2_context.data_start_lba]

    pop ebx
    ret

; FUNC: reads a FAT32 cluster into RAM
; Input:
;   - EAX = Cluster number
;   - ES:SI = Destination buffer in RAM
; Output:
;   - CF = 0 (success), 1 (failure)
stage2_read_cluster:
    push cx
    push edx

    mov dl, [stage2_context.boot_drive]

    call fat32_cluster_to_lba

    movzx cx, byte [stage2_context.bpb + FAT32_BPB_SECTORS_PER_CLUSTER_OFF] ; sectors to read

    call disk_read_lba

    pop edx
    pop cx
    ret

%include "memory.asm"
%include "video.asm"
%include "disk_read.asm"


; --- Data Section & Buffers

; this structure is used by stage2 functions
; stage2_init will populate this structure
stage2_context:
    ; previous-stages data
    .boot_drive:                    db 0
    .mbr_partition_entry:           times 16 db 0
    .bpb:                           times 25 db 0
    .ebpb:                          times 54 db 0

    ; calculated values
    .fat_start_lba:                 dd 0
    .data_start_lba:                dd 0
    .bytes_per_cluster:             dd 0


; Error Strings
msg_err_prefix:                     db 'Boot failed (Stage2): ', 0

msg_err_not_fat32_lba:              db 'ERR_NOT_FAT32_LBA', 0
msg_err_disk_read:                  db 'ERR_DISK_READ', 0
msg_err_boot_indicator:             db 'ERR_BOOT_INDICATOR', 0
msg_err_hidden_sectors_mismatch:    db 'ERR_HIDDEN_SECTORS_MISMATCH', 0 
msg_err_invalid_hidden_sectors:     db 'ERR_INVALID_HIDDEN_SECTORS', 0
msg_err_bytes_per_sector:           db 'ERR_INVALID_BYTES_PER_SECTOR', 0
msg_err_sectors_per_cluster:        db 'ERR_INVALID_SECTORS_PER_CLUSTER', 0
msg_err_number_fats:                db 'ERR_INVALID_NUMBER_FATS', 0
msg_err_root_cluster:               db 'ERR_INVALID_ROOT_CLUSTER', 0
msg_err_total_sectors_16:           db 'ERR_INVALID_TOTAL_SECTORS_16', 0
msg_err_total_sectors_32:           db 'ERR_INVALID_TOTAL_SECTORS_32', 0
msg_err_fat_size_32:                db 'ERR_INVALID_FAT_SIZE_32', 0
