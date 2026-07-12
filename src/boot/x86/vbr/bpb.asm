; FAT32 BPB

bpb_bytes_per_sector:           dw 512
bpb_sectors_per_cluster:        db 4    ; current cluster size: 2048 bytes
bpb_reserved_sectors_count:     dw 32
bpb_number_fats:                db 2
bpb_root_entry_count:           dw 0
bpb_total_sectors_16:           dw 0
bpb_media_descriptor            db 0xF8
bpb_fat_size_16:                dw 0
bpb_sectors_per_track:          dw 63   ; placeholder
bpb_number_of_heads:            dw 255  ; placeholder
bpb_hidden_sectors:             dd 2048
bpb_total_sectors_32:           dd 0x00040000 ; (128 MiB)