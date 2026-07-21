; Memory utilities

; FUNC: copies N bytes from SRC to DST
; Input:
;   - DS:SI = SRC
;   - ES:DI = DST
;   - CX = N
; Output: None
memcpy:
    pusha

    cld     ; clear direction flag (increment addresses)
    rep movsb   ; Repeat CX times: MOVe String Byte (DS:SI -> ES:DI)

    popa

    ret
