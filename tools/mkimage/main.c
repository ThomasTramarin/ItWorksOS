/**
 * @file mkimage.c
 * @brief Generates a bootable FAT32 disk image for x86 BIOS.
 *
 * Format of file <disk.files>:
 * host_path1:image_path1\n
 * host_path2:image_path2\n
 *
 * Note: the image_path must be the absolute path inside the image
 *
 */

#include "fat32.h"
#include "parser.h"
#include "tree.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 5) {
    fprintf(stderr, "mkimage: usage: mkimage <mbr.bin> <vbr.bin> <output.img> "
                    "<disk.files>\n");
    return 1;
  }

  int status = 1;

  FILE *mbr_fd = fopen(argv[1], "rb");
  FILE *vbr_fd = fopen(argv[2], "rb");
  FILE *image_fd = fopen(argv[3], "wb+");
  FILE *files_fd = fopen(argv[4], "rb");

  if (!mbr_fd || !vbr_fd || !image_fd || !files_fd) {
    perror("mkimage");
    if (mbr_fd)
      fclose(mbr_fd);
    if (vbr_fd)
      fclose(vbr_fd);
    if (image_fd)
      fclose(image_fd);
    if (files_fd)
      fclose(files_fd);
    return 1;
  }

  mbr_t mbr;
  fat32_vbr_t vbr;

  fread(&mbr, 1, sizeof(mbr), mbr_fd);
  fread(&vbr, 1, sizeof(vbr), vbr_fd);

  // initialize disk image with bytes set to 0x00
  uint64_t partition_offset =
      (uint64_t)mbr.mbr_entry[0].lba_start * vbr.bytes_per_sector;

  uint64_t image_size =
      partition_offset + (uint64_t)vbr.total_sectors_32 * vbr.bytes_per_sector;

  fseek(image_fd, image_size - 1, SEEK_SET);
  fputc(0, image_fd);
  rewind(image_fd);

  // write MBR at the first disk sector
  fwrite(&mbr, 1, sizeof(mbr), image_fd);

  // write VBR at the first partition sector
  fseek(image_fd, partition_offset, SEEK_SET);
  fwrite(&vbr, 1, sizeof(vbr), image_fd);

  // fat32 context creation
  ctx_t ctx = {
      .image = image_fd,
      .vbr = &vbr,
      .partition_offset = partition_offset,
      .root = parse_disk_files(files_fd),
  };

  if (!ctx.root) {
    fprintf(stderr, "mkimage: failed to parse disk files\n");
    goto cleanup;
  }

  // initialize FAT table
  if (!fat32_table_init(&ctx)) {
    fprintf(stderr, "mkimage: failed to initialize FAT table\n");
    fs_node_free(ctx.root);
    goto cleanup;
  }

  // assign clusters to directories
  fat32_prepare_tree(&ctx, ctx.root);

  // write files and directories
  fat32_write_tree(&ctx, ctx.root);

  // write FAT table
  fat32_table_flush(&ctx);

  status = 0;

  free(ctx.fat);
  fs_node_free(ctx.root);

cleanup:
  fclose(mbr_fd);
  fclose(vbr_fd);
  fclose(image_fd);
  fclose(files_fd);

  return status;
}
