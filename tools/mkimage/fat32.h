#ifndef FAT32_H
#define FAT32_H

#include "tree.h"
#include <stdint.h>
#include <stdio.h>

#define FAT_NAME_LEN 11

typedef struct {
  uint8_t status;
  uint8_t chs_start[3];
  uint8_t partition_type;
  uint8_t chs_end[3];
  uint32_t lba_start;
  uint32_t sector_count;
} __attribute__((packed)) mbr_partition_entry_t;

typedef struct {
  uint8_t boot_code[446];
  mbr_partition_entry_t mbr_entry[4];
  uint16_t mbr_signature;
} mbr_t;

typedef struct {
  uint8_t jmp_boot[3];
  uint8_t oem_name[8];

  // BPB Standard (BIOS Parameter Block)
  uint16_t bytes_per_sector;
  uint8_t sectors_per_cluster;
  uint16_t reserved_sectors;
  uint8_t fat_count;
  uint16_t root_entry_count;
  uint16_t total_sectors_16;
  uint8_t media_descriptor;
  uint16_t fat_size_16;
  uint16_t sectors_per_track;
  uint16_t head_count;
  uint32_t hidden_sectors;
  uint32_t total_sectors_32;

  // Extended BPB (FAT32)
  uint32_t fat_size_32;
  uint16_t ext_flags;
  uint16_t fs_version;
  uint32_t root_cluster;
  uint16_t fsinfo_sector;
  uint16_t backup_boot_sector;
  uint8_t reserved[12];
  uint8_t drive_number;
  uint8_t reserved1;
  uint8_t boot_signature;
  uint32_t volume_id;
  uint8_t volume_label[11];
  uint8_t fs_type[8];

  uint8_t boot_code[420];
  uint16_t boot_signature_vbr;
} __attribute__((packed)) fat32_vbr_t;

typedef struct {
  uint32_t lead_signature;     // 0x41415252
  uint8_t reserved1[480];      // 0x00
  uint32_t struct_signature;   // 0x61417272
  uint32_t free_cluster_count; // 0xFFFFFFFF (unknown)
  uint32_t next_free_cluster;
  uint8_t reserved2[12];    // 0x00
  uint32_t trail_signature; // 0xAA550000
} __attribute__((packed)) fat32_fsinfo_t;

typedef struct {
  uint8_t name[11];
  uint8_t attr;
  uint8_t nt_res;
  uint8_t crt_time_tenth;
  uint16_t crt_time;
  uint16_t crt_date;
  uint16_t lst_acc_date;
  uint16_t fst_clus_hi;
  uint16_t wrt_time;
  uint16_t wrt_date;
  uint16_t fst_clus_lo;
  uint32_t file_size;
} __attribute__((packed)) fat_dir_entry_t;

// file entry attributes
#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20

// FAT table next cluster types
#define FAT_FREE 0x00
#define FAT_EOF 0x0FFFFFFF
#define FAT_BAD_CLUSTER 0x0FFFFFF7

typedef struct {

  FILE *image;

  fat32_vbr_t *vbr;

  uint32_t *fat;

  fat32_fsinfo_t *fsinfo;

  fs_node_t *root;

  uint64_t partition_offset;

  uint32_t next_free_cluster;
} ctx_t;

int fat32_table_init(ctx_t *ctx);
uint32_t fat32_cluster_alloc(ctx_t *ctx);
void fat32_table_flush(ctx_t *ctx);
void fat32_prepare_tree(ctx_t *ctx, fs_node_t *node);
void fat32_write_tree(ctx_t *ctx, fs_node_t *node);

#endif