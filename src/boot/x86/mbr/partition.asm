
; FUNC
; Input:
;   DS:SI: pointer to the MBR partition table (address of the first entry)
; Output:
;   AX: offset of active partition entry:
;       0, 16, 32, 48 -> Bootable partition found, the value represents
;                        the offset of the active MBR entry in the MBR partition table
;       64 -> no bootable partition found
find_active_partition:
    push bx
    push cx
    push si

    xor bx, bx ; bx = 0
    xor cx, cx ; cx = 0

.loop_entries:

    cmp byte [ds:si + bx], 0x80
    je .found

    inc cx
    cmp cx, 4
    je .not_found

    add bx, 16
    jmp .loop_entries

.found:
    mov ax, bx
    jmp .done

.not_found:
    mov ax, 64

.done:
    pop si
    pop cx
    pop bx
    ret
