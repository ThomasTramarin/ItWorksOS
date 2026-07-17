; FAT32 BPB

bpb_bytes_per_sector:           dw 512
bpb_sectors_per_cluster:        db 1            ; 1 sector = 1 cluster (512 bytes)
bpb_reserved_sectors_count:     dw 32           ; 32 reserved sectors before the first FAT table
bpb_number_fats:                db 2            ; 2 FAT tables
bpb_root_entry_count:           dw 0
bpb_total_sectors_16:           dw 0
bpb_media_descriptor            db 0xF8
bpb_fat_size_16:                dw 0
bpb_sectors_per_track:          dw 63           ; geometry placeholder
bpb_number_of_heads:            dw 255          ; geometry placeholder
bpb_hidden_sectors:             dd 2048         ; the same value of mbr1_lba_start
bpb_total_sectors_32:           dd 0x00020000   ; (64 MiB)