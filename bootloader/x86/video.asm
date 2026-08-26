[bits 16]

; FUNC: print a string 
;
; Input:
;   DS:SI = pointer to the string to print
; Output: none
video_print_string:
    pusha

    mov ah, 0x0E ; Teletype Ouptut BIOS call
    mov bh, 0x0  ; page number

.loop:
    lodsb ; load a byte from SI into AL, then increment SI
    cmp al, 0x0 ; check for NULL term
    je .done

    ; AL contains the character to print
    int 0x10
    jmp .loop

.done:
    popa
    ret