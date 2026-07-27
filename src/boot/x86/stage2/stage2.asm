[org 0x0700]
[bits 16]

; Stage2 Bootloader Code
; Responsibilities:
;   - Save previous boot stages information
;   - Validate the format of MBR partition and VBR parameters 
;   - Loads /BOOT/KERNEL.BIN
;   - Loads GDT
;   - Enters Protected Mode
;   - Prepares boot_info structure
;   - Transfer execution to the kernel code

%include "constants.inc"

; --- Entry Point ---
stage2_entry:
    call stage2_init
    call stage2_validate

    ; --- Load kernel ---
    mov eax, [stage2_context.ebpb + FAT32_EBPB_ROOT_CLUSTER_OFF]
    mov si, boot_dir
    call fat32_find_entry
    jc .err_disk_read

    xor eax, eax
    mov ax, [buf_dir + bx + FAT32_DIR_CLUSTER_HIGH_OFF]
    shl eax, 16
    mov ax, [buf_dir + bx + FAT32_DIR_CLUSTER_LOW_OFF]

    mov si, kernel_file
    call fat32_find_entry
    jc .err_disk_read

    xor eax, eax
    mov ax, [buf_dir + bx + FAT32_DIR_CLUSTER_HIGH_OFF]
    shl eax, 16
    mov ax, [buf_dir + bx + FAT32_DIR_CLUSTER_LOW_OFF]

    ; EAX = first kernel.bin data cluster
    mov bx, KERNEL_LOAD_SEG
    mov es, bx
    mov si, KERNEL_LOAD_OFF
    call fat32_load_file
    jc .err_disk_read

    ; save memory map into memory
    xor ax, ax
    mov es, ax
    mov di, e820_buffer
    call e820_get_memory_map
    jc .err_e820
    mov [e820_count], cx

    call stage2_boot_info_init

    jmp stage2_enter_protected_mode

.err_disk_read:
    mov si, msg_err_disk_read
    jmp stage2_error

.err_e820:
    mov si, msg_err_e820
    jmp stage2_error

.halt:
    hlt
    jmp .halt


; --- Functions ---

; FUNC: initializes the stage2_context structure
; Input:
;   - DL = boot drive
;   - DS:SI = MBR boot partition entry
;   - ES:DI = VBR pointer
; Output: stage2_context populated
stage2_init:
    pushad

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

    popad
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
    pushad

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

    popad
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

; FUNC
; Input:
;   - EAX = cluster number
; Output:
;   - EAX = next cluster
;       Free cluster: 0x00000000
;       Bad cluster: 0x0FFFFFF7
;       EOF: 0xFFFFFF8 - 0x0FFFFFFF
;   - CF = 0 (success), 1 (failure) -> disk read error
fat32_read_fat_entry:
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp

    xor edx, edx    ; EDX = 0

    ; each FAT entry is 32-bit
    ; a FAT sector can contain 128 FAT entries
    mov ecx, 128
    div ecx     ; EAX / ECX = EAX (quotient), EDX (remaider)

    mov ecx, [stage2_context.fat_start_lba]
    add eax, ecx    ; EAX (fat sector to read) = EAX + ECX

    push edx    ; save remainder

    mov si, buf_fat
    mov cx, 1
    mov dl, [stage2_context.boot_drive]

    push es ; save ES (used by fat32_load_file)
    push ds
    pop es  ; set ES = DS (read correctly in DS:buf_fat)

    call disk_read_lba

    pop es  ; restore ES

    pop edx ; restore edx (remainder)
    jc .err

    imul edx, 4 ; EDX = the offset of the FAT entry

    mov eax, [buf_fat + edx]    ; EDX = next cluster
    and eax, 0x0FFFFFFF         ; do not consider the upper 4 bits (reserved)

    jmp .done

.err:
    stc ; force CF = 1

.done:
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    ret

; FUNC: find a file/directory by an 8.3 name inside a directory
; Input:
;   EAX = starting directory cluster
;   DS:SI = pointer to the string name (11 bytes, 8.3 format)
; Output:
;   BX = offset of the found entry relative to buf_dir (valid until next read on buf_dir)
;   CF = 0 (found), 1 (not found/disk_error)
fat32_find_entry:
    push eax
    push ecx
    push edx
    push esi
    push edi
    push ebp


    sub esp, 12  ; reserve space on the stack to store local variables
    mov ebp, esp

    ; Local Variables (12 bytes):
    ;   - ebp + 0 (2 bytes) = SI pointer to the 8.3 string
    ;   - ebp + 2 (1 byte) = sectors_left (sectors_per_cluster can be max 128)
    ;   - ebp + 3 (1 byte) = alignment
    ;   - ebp + 4 (4 bytes) = current_cluster 
    ;   - ebp + 8 (4 bytes) = current_lba

    mov word [ebp + 0], si
    mov [ebp + 4], eax

; while(next_dir_cluster != EOF)
.cluster_loop:
    mov eax, [ebp + 4]
    call fat32_cluster_to_lba   ; returns EAX = starting LBA sector

    ; initialize the remaining sectors counter for this cluster
    movzx cx, byte [stage2_context.bpb + FAT32_BPB_SECTORS_PER_CLUSTER_OFF]
    mov [ebp + 2], cl

; for each sector of the current_cluster
.sector_loop:
    mov [ebp + 8], eax   ; save current_lba

    ; read one sector into buf_dir
    mov cx, 1
    mov dl, [stage2_context.boot_drive]
    mov si, buf_dir

    ; ensure ES = DS
    push ds
    pop es

    call disk_read_lba
    jc .not_found   ; disk error

    ; iterate through all 16 entries in this sector (512 bytes / 32 bytes = 16 entries per sector)
    mov bx, buf_dir
    mov cx, 16  

; for each directory entry of this sector
.entry_loop:
    ; End Of Directory check (if the entry starts with 0x00)
    cmp byte [bx + FAT32_DIR_NAME_OFF], 0x00
    je .not_found   ; not found -> no other entries

    ; Deleted entry check (if the entry starts with 0xE5)
    cmp byte [bx + FAT32_DIR_NAME_OFF], 0xE5
    je .next_entry  ; do not consider this entry (garbage information) -> go to the next entry

    ; Read Attribute byte
    mov al, [bx + FAT32_DIR_ATTRIBUTES_OFF]
    cmp al, 0x0F    
    je .next_entry  ; skip LFN entries

    test al, 0x08   
    jnz .next_entry ; skip Volume ID entries (if the bit is set)

    ; compare the name
    push cx ; save entries left
    push bx ; save current entry offset

    mov si, [ebp + 0]
    mov di, bx
    mov cx, 11 ; name length
    repe cmpsb  ; compare ES:DI and DS:SI

    pop bx
    pop cx
    je .found   ; match found

.next_entry:
    add bx, 32  ; advance to the next entry
    loop .entry_loop    ; next entry

    ; move to the next sector in the current cluster
    mov eax, [ebp + 8]   ; get current_lba
    inc eax             ; next sector
    dec byte [ebp + 2]  ; decrement sectors_left
    jnz .sector_loop 

    ; move to the next cluster in the FAT chain
    mov eax, [ebp + 4]   ; get the current cluster
    call fat32_read_fat_entry   ; returns EAX = next cluster (or EOF/bad cluster)
    jc .not_found

    ; check for EOF (0x0FFFFFF8 - 0x0FFFFFFF) or bad cluster (0x0FFFFFF7)
    cmp eax, 0x0FFFFFF7
    jae .not_found  ; reached EOF/bad cluster without finding file

    mov [ebp + 4], eax   ; store new cluster
    jmp .cluster_loop

.found:
    add esp, 12  ; free local stack frame
    sub bx, buf_dir

    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop eax

    ; BX contains the offset of the found entry

    clc         ; force CF = 0
    ret


.not_found:
    add esp, 12  ; free local stack frame


    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop eax 

    stc         ; force CF = 1
    ret

; FUNC: load a file into memory at a specific memory location
; Input: 
;   - EAX = first cluster of file
;   - ES:SI = destination address. Real mode address (segment + offset)
; Output:
;   - CF = 0 (success), 1 (failure)
;   - ES:SI = address immediately after loaded data
fat32_load_file:
    push eax
    push ebx

    mov ebx, eax    ; current cluster

.load_loop:
    ; read the current cluster in ES:SI
    mov eax, ebx
    call stage2_read_cluster
    jc .err

    ; increment the buffer of bytes_per_cluster

    ; bytes_per_cluster is stored as DWORD
    ; FAT32 cluster sizes fit in 16 bits for normal configurations
    ; but the value is kept 32-bit
    mov eax, [stage2_context.bytes_per_cluster]
    add si, ax

    ; if SI overflows (16-bit), advance the segment
    ; by 0x1000 (64 KiB) to keep a continuous linear address
    jnc .no_segment_wrap

    mov ax, es
    add ax, 0x1000
    mov es, ax

.no_segment_wrap:

    ; find the next cluster
    mov eax, ebx
    call fat32_read_fat_entry
    jc .err

    ; EAX = next cluster

    cmp eax, 0x0FFFFFF7
    je .err

    cmp eax, 0x0FFFFFF8
    jae .done

    mov ebx, eax
    jmp .load_loop

.err:
    stc ; force CF = 1

    pop ebx
    pop eax

    ret
.done:
    clc ; force CF = 0

    pop ebx
    pop eax

    ret

; FUNC: initializes boot_info structure stored on stage2 data memory
; Input: None
; Output: None
stage2_boot_info_init:
    push ax

    ; e820 count
    mov ax, [e820_count]
    mov [boot_info.memory_map_count], ax

    ; e820 pointer
    mov dword [boot_info.memory_map_ptr], e820_buffer

    pop ax
    ret


%include "memory.asm"
%include "video.asm"
%include "disk_read.asm"
%include "e820.asm"



stage2_enter_protected_mode:
    cli ; disable interrupts

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

    jmp GDT_CODE_OFFSET:protected_mode_main

[bits 32]
protected_mode_main:

    ; update all segment registers to use the new Data Segment (0x10)
    mov ax, GDT_DATA_OFFSET
    mov ds, ax 
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov gs, ax

    ; Stack setup
    mov ebp, STACK_ADDR
    mov esp, ebp


    ; save magic on eax ('IWBT' in little-endian)
    mov eax, BOOT_MAGIC

    ; save boot_info on ebx, then kernel.asm will pass it
    mov ebx, boot_info 

    ; far jump into the compiled kernel
    jmp GDT_CODE_OFFSET:KERNEL_LOAD_ADDR


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
msg_err_e820:                       db 'ERR_E820_MEMORY_MAP'

; FAT32 filenames
boot_dir: db 'BOOT       '
kernel_file: db 'KERNEL  BIN' 

; Buffers
buf_fat:                            times 512 db 0   ; 1-sector buffer to store a FAT sector 
buf_dir:                            times 512 db 0   ; 1-sector buffer to store directory entries

; GDT implementation
gdt_start:
    ; NULL descriptor
    dd 0x0
    dd 0x0

    ; Code Segment Descriptor (offset: 0x8)
    dw 0xFFFF     ; limit[15:0] (combined with upper limit bits = 4GB)
    dw 0x0000     ; base[15:0] (segment starts at address 0x0)
    db 0x00       ; base[23:16] 
    db 10011010b  ; access byte: present, ring 0, code segment, readable
    db 11001111b  ; flags: 4KB granularity, 32-bit segment, limit[19:16]
    db 0x00       ; base[32:24]

    ; Data Segment Descriptor (offset: 0x10)
    dw 0xFFFF     ; limit[15:0] (combined with upper limit bits = 4GB)
    dw 0x0000     ; base[15:0] (segment starts at address 0x0)
    db 0x00       ; base[23:16] 
    db 10010010b  ; access byte: present, ring 0, data segment, writable
    db 11001111b  ; flags: 4KB granularity, 32-bit segment, limit[19:16]
    db 0x00       ; base[32:24]

gdt_end:

; GDT Register Pointer structure needed by the 'lgdt' instruction
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

e820_buffer:
    times (64 * 24) db 0    ; 64 = max number of E820 supported entries, 24 = entry size
e820_count:
    dw 0

; boot_info structure passed to the kmain function as a pointer
boot_info:
    .memory_map_count:    dw 0        ; E820 entries count (2 bytes)
    .memory_map_ptr:   dd 0        ; physical RAM pointer to the first E820 entry (4 bytes)
