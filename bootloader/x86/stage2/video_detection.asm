
; Func
; Input:
;   ES:DI = destination pointer to 'struct boot_video' (16 bytes)
; Output:
;   CF = 0 (success), 1 (failure)
;   ES:DI populated
video_detect_mode:
    pusha

    mov ah, 0x0F ; get video mode BIOS routine
    int 10h

    ; currently, only mode 3 is supported (text mode VGA 80x25)
    cmp al, 0x03    ; AL contains the mode
    je .mode_text_03h

    stc ; Mode not supported
    jmp .end

.mode_text_03h:

    mov dword [es:di], 1   ; BOOT_VIDEO_TYPE_TEXT

    mov byte [es:di + 4], al ; mode
    mov byte [es:di + 5], bh ; page
    mov byte [es:di + 6], ah ; cols
    mov byte [es:di + 7], 25 ; rows

    mov dword [es:di + 8], 0x000B8000
    mov dword [es:di + 12], 0x0
    

    clc
.end:
    popa
    ret

