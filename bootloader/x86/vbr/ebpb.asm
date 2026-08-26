; FAT32 Extended BPB

ebpb_fat_size_32:           dd 1024             ; 1024 sectors (512KiB) per FAT table
ebpb_extended_flags:        dw 0                ; FAT mirroring enabled
ebpb_fs_version:            dw 0
ebpb_root_cluster:          dd 2                ; root directory = cluster 2   
ebpb_fs_info_sector:        dw 1
ebpb_backup_boot_sector:    dw 6
ebpb_reserved1:             dd 0x00, 0x00, 0x00 ; (12 bytes)
ebpb_drive_number:          db 0x80
ebpb_reserved2:             db 0x00
ebpb_boot_signature:        db 0x29
ebpb_vol_id:                dd 0x12345678
ebpb_vol_label:             db 'IWOMHOS    '
ebpb_fs_type:               db 'FAT32   '