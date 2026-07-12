mbr1_type: db 0x80                      ; active partition
mbr1_chs_start: db 0xFE, 0xFF, 0xFF     ; Start CHS (placeholder)
mbr1_partition_type: db 0x0C            ; FAT32 LBA
mbr1_chs_end: db 0xFE, 0xFF, 0xFF       ; END CHS (placeholder)
mbr1_lba_start: dd 0x00000800           ; LBA address (2048)
mbr1_num_sectors: dd 0x00040000         ; Number of sectors: 262144 (128 Mib)

times 48 db 0                           ; Fill empty entries 