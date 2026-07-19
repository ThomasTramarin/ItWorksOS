[org 0x0700]
[bits 16]

mov si, msg
call video_print_string


;data
msg: db 'Hello From Stage 2',0xD, 0xA, 0

%include "video.asm"