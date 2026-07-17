#include "fat32.h"
#include "tree.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Utilities
 */
static uint32_t fat32_cluster_size(ctx_t *ctx) {
  return ctx->vbr->bytes_per_sector * ctx->vbr->sectors_per_cluster;
}

static uint64_t fat32_data_offset(ctx_t *ctx) {
  return ctx->partition_offset +
         ((uint64_t)ctx->vbr->reserved_sectors +
          ((uint64_t)ctx->vbr->fat_count * ctx->vbr->fat_size_32)) *
             ctx->vbr->bytes_per_sector;
}

/**
 * FAT table
 */
int fat32_table_init(ctx_t *ctx) {
  if (!ctx || !ctx->vbr)
    return 0;

  fat32_vbr_t *vbr = ctx->vbr;

  uint32_t entries =
      (vbr->fat_size_32 * vbr->bytes_per_sector) / sizeof(uint32_t);

  ctx->fat = calloc(entries, sizeof(uint32_t));

  if (!ctx->fat)
    return 0;

  // the first two entries are reserved
  ctx->fat[0] = 0xFFFFFF00 | vbr->media_descriptor;
  ctx->fat[1] = FAT_EOF;
  ctx->fat[vbr->root_cluster] = FAT_EOF;

  ctx->next_free_cluster = vbr->root_cluster + 1;

  return 1;
}

// finds the first free cluster
uint32_t fat32_cluster_alloc(ctx_t *ctx) {

  uint32_t total_entries =
      (ctx->vbr->fat_size_32 * ctx->vbr->bytes_per_sector) / sizeof(uint32_t);

  for (uint32_t i = ctx->next_free_cluster; i < total_entries; i++) {

    if (ctx->fat[i] == FAT_FREE) {
      ctx->fat[i] = FAT_EOF;

      ctx->next_free_cluster = i + 1;

      return i;
    }
  }

  return 0;
}

void fat32_table_flush(ctx_t *ctx) {
  uint64_t fat_size =
      (uint64_t)ctx->vbr->fat_size_32 * ctx->vbr->bytes_per_sector;

  // offset of the first FAT table
  uint64_t first_fat =
      ctx->partition_offset +
      (uint64_t)ctx->vbr->reserved_sectors * ctx->vbr->bytes_per_sector;

  for (uint8_t i = 0; i < ctx->vbr->fat_count; i++) {
    fseek(ctx->image, first_fat + i * fat_size, SEEK_SET);

    fwrite(ctx->fat, 1, fat_size, ctx->image);
  }
}

/**
 * Cluster Write
 */
void fat32_write_cluster(ctx_t *ctx, uint32_t cluster, void *buffer) {
  uint64_t offset = fat32_data_offset(ctx) +
                    ((uint64_t)(cluster - 2) * fat32_cluster_size(ctx));

  fseek(ctx->image, offset, SEEK_SET);

  fwrite(buffer, 1, fat32_cluster_size(ctx), ctx->image);
}

/**
 * File Writing
 */
void fat32_write_file(ctx_t *ctx, fs_node_t *node) {
  FILE *fp = fopen(node->file.host_path, "rb");

  if (!fp)
    return;

  uint32_t cluster_size = fat32_cluster_size(ctx);

  uint32_t remaining = node->file.size;

  uint32_t cluster = node->first_cluster;
  uint32_t previous = 0;

  uint8_t *buffer = calloc(1, cluster_size);

  while (remaining > 0) {

    if (previous != 0)
      ctx->fat[previous] = cluster;

    memset(buffer, 0, cluster_size);

    fread(buffer, 1, cluster_size, fp);

    fat32_write_cluster(ctx, cluster, buffer);

    remaining -= remaining > cluster_size ? cluster_size : remaining;

    previous = cluster;

    if (remaining > 0)
      cluster = fat32_cluster_alloc(ctx);
  }

  ctx->fat[previous] = FAT_EOF;

  free(buffer);
  fclose(fp);
}

/*
 * Directory entry
 */

static void fat32_create_entry(fat_dir_entry_t *entry, fs_node_t *node) {

  memset(entry, 0, sizeof(*entry));

  /*
   * simplified 8.3 name
   * No LFN support
   */
  char tmp[FAT_NAME_LEN];
  memset(tmp, ' ', sizeof(tmp)); // fill with empty spaces

  char name[FAT_NAME_LEN];
  strcpy(name, node->name);

  char *dot = strchr(name, '.');

  if (dot) {
    *dot = '\0';
    memcpy(tmp, name, strlen(name));
    memcpy(tmp + 8, dot + 1, strlen(dot + 1));
  } else {
    memcpy(tmp, name, strlen(name));
  }

  // transform characters to uppercase
  for (size_t i = 0; i < FAT_NAME_LEN; i++) {
    tmp[i] = toupper(tmp[i]);
  }

  memcpy(entry->name, tmp, FAT_NAME_LEN);

  entry->attr = (node->type == NODE_DIRECTORY) ? ATTR_DIRECTORY : ATTR_ARCHIVE;

  entry->fst_clus_hi = node->first_cluster >> 16;

  entry->fst_clus_lo = node->first_cluster & 0xffff;

  if (node->type == NODE_FILE)
    entry->file_size = node->file.size;
}

/**
 * Directory Write
 */
void fat32_write_directory(ctx_t *ctx, fs_node_t *node) {
  uint32_t cluster_size = fat32_cluster_size(ctx);

  uint8_t *buffer = calloc(1, cluster_size);

  if (!buffer)
    return;

  fat_dir_entry_t *entries = (fat_dir_entry_t *)buffer;

  uint32_t index = 0;

  int is_root = (node == ctx->root);

  if (!is_root) {
    /**
     * Directory: . (current)
     */
    memset(&entries[index], 0, sizeof(fat_dir_entry_t));
    memcpy(entries[index].name, ".          ", 11);
    entries[index].attr = ATTR_DIRECTORY;
    entries[index].fst_clus_hi = node->first_cluster >> 16;

    entries[index].fst_clus_lo = node->first_cluster & 0xffff;

    index++;

    /**
     * Directory: .. (parent)
     */
    memset(&entries[index], 0, sizeof(fat_dir_entry_t));
    memcpy(entries[index].name, "..         ", 11);
    entries[index].attr = ATTR_DIRECTORY;

    if (node->parent) {
      uint32_t parent = node->parent->first_cluster;

      entries[index].fst_clus_hi = parent >> 16;

      entries[index].fst_clus_lo = parent & 0xffff;
    }
    index++;
  }

  /**
   * Children nodes
   *
   */
  for (uint32_t i = 0; i < node->dir.child_count; i++) {
    fat32_create_entry(&entries[index], node->dir.children[i]);

    index++;
  }

  fat32_write_cluster(ctx, node->first_cluster, buffer);

  free(buffer);
}

/**
 * First Step:
 * allocate clusters
 */
void fat32_prepare_tree(ctx_t *ctx, fs_node_t *node) {
  if (node == ctx->root)
    node->first_cluster = ctx->vbr->root_cluster;
  else if (node->first_cluster == 0)
    node->first_cluster = fat32_cluster_alloc(ctx);

  if (node->type == NODE_DIRECTORY) {

    for (uint32_t i = 0; i < node->dir.child_count; i++) {
      fat32_prepare_tree(ctx, node->dir.children[i]);
    }
  }
}

/**
 * Second Step:
 * write data
 */
void fat32_write_tree(ctx_t *ctx, fs_node_t *node) {
  if (node->type == NODE_FILE) {
    fat32_write_file(ctx, node);
    return;
  }
  fat32_write_directory(ctx, node);

  for (uint32_t i = 0; i < node->dir.child_count; i++) {
    fat32_write_tree(ctx, node->dir.children[i]);
  }
}