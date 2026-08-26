[org 0x0500]
[bits 16]

jmp_istr: 
    jmp boot_code
    nop

oem_name: db 'IWOMHOS '
%include "vbr/bpb.asm"
%include "vbr/ebpb.asm"

; The VBR has to:
;   - find the stage2 bootloader at `/boot/stage2.bin`
;   - load it into a safe memory area
;   - pass the control to stage 2
boot_code:
    xor ax, ax
    mov ds, ax
    mov es, ax

    mov [boot_drive], dl
    mov [mbr_boot_partition_ptr], si

    ; FAT start sector = partition_start + reserved_sectors
    mov eax, [bpb_hidden_sectors]
    movzx ebx, word [bpb_reserved_sectors_count]
    add eax, ebx    ; eax contains the first sector of the first FAT table
    mov [fat_start_sector], eax

    ; Data start sector = FAT_start_sector + (fat_count * fat_size_32)
    movzx ebx, byte [bpb_number_fats]
    imul ebx, [ebpb_fat_size_32]
    add eax, ebx    ; eax contains the data start sector
    mov [data_start_sector], eax

    ; load the root directory
    mov eax, [ebpb_root_cluster]
    call load_cluster
    jc .halt

    ; find BOOT directory
    mov di, boot_dir
    call find_entry
    jc .halt

    ; now DS:SI points to the entry
    mov ax, [si + DIR_ENTRY_FIRST_CLUS_HI]
    shl eax, 16
    mov ax, [si + DIR_ENTRY_FIRST_CLUS_LO]

    ; load the BOOT/ directory cluster
    call load_cluster
    jc .halt

    ; find stage2.bin
    mov di, stage2_file
    call find_entry
    jc .halt 

    ; now DS:SI points to the entry
    mov ax, [si + DIR_ENTRY_FIRST_CLUS_HI]
    shl eax, 16
    mov ax, [si + DIR_ENTRY_FIRST_CLUS_LO]
    ; EAX contains the first cluster number of stage2.bin

.load_stage2_loop:
    call load_cluster
    jc .halt

    ; increment the load address
    mov bl, [bpb_sectors_per_cluster]
    shl bl, 1
    add [load_ptr + 1], bl

    mov dl, [boot_drive]
    call get_next_cluster
    jc .halt
    cmp eax, 0x0FFFFFF8
    jb .load_stage2_loop
    ; EOC (End Of Chain)
    mov si, [mbr_boot_partition_ptr]
    mov di, 0x0500
    jmp 0x0000:CLUSTER_LOAD_OFF

.halt:
    hlt
    jmp .halt

; data
boot_drive: db 0
mbr_boot_partition_ptr: dw 0


fat_start_sector dd 0
data_start_sector dd 0
current_cluster dd 0
load_ptr dw CLUSTER_LOAD_OFF

boot_dir:    db 'BOOT       '
stage2_file: db 'STAGE2  BIN'

; constants
CLUSTER_LOAD_OFF equ 0x0700
FAT_SEC_LOAD_OFF equ 0x7E00
DIR_ENTRY_FIRST_CLUS_HI equ 0x14
DIR_ENTRY_FIRST_CLUS_LO equ 0x1A

; FUNC
; Input:
;   - eax = cluster
; Output:
;   - eax = sector
cluster_to_sector:
    push ebx
    push edx

    sub eax,2

    movzx ebx, byte [bpb_sectors_per_cluster]

    mul ebx ; EDX:EAX = EAX * EBX

    add eax,[data_start_sector]

    pop edx
    pop ebx
    ret

; FUNC: loads one cluster at 0x0000:CLUSTER_LOAD_OFF
; Input:
;   - EAX: cluster number
;   - DL = drive number
; Output:
;   - CF = 0 (success), 1 (failed)
load_cluster:
    push eax
    push bx
    push cx
    push si

    call cluster_to_sector ; EAX (cluster) -> EAX (LBA)

    xor bx, bx
    mov es, bx
    mov si, [load_ptr]

    movzx cx, byte [bpb_sectors_per_cluster]
    call disk_read_lba

    pop si
    pop cx
    pop bx
    pop eax
    ret

; FUNC: find an 8.3 entry in the directory currently loaded at CLUSTER_LOAD_OFF
; Input:
;   - ES:DI = pointer to the entry string to find (8.3 formatted)
; Output:
;   - CF = 0 (found), 1 (not found)
;   - DS:SI = pointer to the entry 
find_entry:
    ; each dir entry is 32 bytes and the name is stored at offset 0x00
    ; this means that a 512-byte sector contains max 16 entries 

    mov si, CLUSTER_LOAD_OFF
    movzx cx, byte [bpb_sectors_per_cluster]
    shl cx, 4   ; left shift 4 positions = cx * 16
    ; now cx contains the number of entries
    cld         ; clear DF (Direction Flag)

.loop_through_entries:
    cmp byte [si], 0x00
    je .not_found

    push cx 
    push si
    push di

    mov cx, 11 ; 8.3, string length
    repe cmpsb  ; REPeat while Equal CoMPare String Byte
                ; compares DS:SI with ES:DI, then SI++ and DI++

    pop di
    pop si
    pop cx

    je .found   ; if cmpsb finished with ZF = 1 (equal), we found the entry


    add si, 32                  ; next entry
    loop .loop_through_entries  ; for (i = CX; i>0; i--)

.not_found:
    stc     ; CF = 1 -> not found
    ret
.found:
    clc
    ret     ; FC  0 -> found
            ; DS:SI points to the entry start

; FUNC: obtain the next cluster of the FAT table
; Input:
;   - EAX = current cluster
;   - DL = drive number
; Output:
;   - EAX = the next cluster of the cluster chain
;   - CF = 0 (success), 1 (failed)
get_next_cluster:
    push ebx
    push ecx
    push edx
    push esi

    mov cl, dl  ; save the drive number on CL

    ; in FAT32 each entry is 32-bytes long. A sector in the FAT region constains 512/4 = 128 entries
    xor edx, edx
    mov ebx, 128
    div ebx ; performs EDX:EAX / EBX = EAX (quotient), EDX (remainder)

    shl edx, 2 ; EDX = EDX * 4
    mov ebx, edx

    mov dl, cl  ; DL = drive number
    mov cx, 1
    add eax, [fat_start_sector]

    xor esi, esi
    mov esi, FAT_SEC_LOAD_OFF
    call disk_read_lba  ; sets the CF if err
    jc .exit

    ; EBX contains the offset from the loaded sector of the next cluster
    mov eax, [esi + ebx] ; EAX now stores the next cluster value
    clc     ; CF = 0 (success)

.exit:
    pop esi
    pop edx
    pop ecx
    pop ebx
    ret

%include "disk_read.asm"

times 510 - ($-$$) db 0
dw 0xAA55