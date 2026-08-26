
; Func
; Input:
;   - ES:DI = destination buffer for 24 byte entries
; Output:
;   - CF = 0 (success), 1 (error)
;   - CX = entry count
;   - ES:DI = pointer to the next free buffer position
e820_get_memory_map:
    push eax
    push ebx
    push edx
    push bp ; BP used as entry counter

    xor ebx, ebx    ; first call must start with EBX = 0
    xor bp, bp

.next_entry:
    mov eax, 0xE820
    mov edx, 0x534D4150 ; magic value ("SMAP")
    mov ecx, 24         ; buffer size (24 bytes, ACPI 3.x)
    mov dword [es:di + 20], 1	; force a default value for ACPI 3.x (bit 0 = 1)
    int 0x15

    ; if CF = 1: end of list or error at the first call
    jc .check_end    

    cmp eax, 0x0534D4150
    jne .error   ; if EAX != "SMAP", the function is not supported by the BIOS

    inc bp
    add di, 24  ; move buffer to next entry

    ; if EBX == 0, this is the last entry
    test ebx, ebx
    jz .success

    jmp .next_entry

.check_end:
    ; if CF = 1 and BP > 0:
    ;   end of list
    test bp, bp
    jnz .success

    ; if CF == 1 and BP == 0:
    ;   error case
    jmp .error

.success:
    mov cx, bp
    clc ; force CF = 0
    jmp .exit

.error:
    stc ; force CF = 1

.exit:
    pop bp
    pop edx
    pop ebx
    pop eax
    ret