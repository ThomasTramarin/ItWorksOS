#ifndef EXEC_IWBF_H
#define EXEC_IWBF_H

/**
 * @brief IWBF (ItWorks Binary Format)
 *
 * IWBF is the binary format used by ItWorksOS for executable
 * user space programs.
 *
 * Byte order:
 *   All multi-byte integers are encoded little-endian.
 *
 * Target:
 *   IWBF currently defines a 32-bit IA-32 executable format.
 *
 * File Layout:
 *
 * +----------------+
 * | IWBF header    |
 * +----------------+
 * | Segment #1     |
 * +----------------+
 * | Segment #2     |
 * +----------------+
 * | Segment #...   |
 * +----------------+
 * | Segment data   |
 * +----------------+
 *
 * The segment table immediately follows the header and contains
 * segment_count consecutive segment descriptors.
 *
 * Segment data is stored at the file offset specified by the corresponding
 * segment descriptor.
 */

#include <base/stdint.h>
#include <stddef.h>

#define IWBF_HDR_MAGIC "IWBF"

/* File Type */
#define IWBF_HDR_TYPE_EXEC 1 // executable binary

/* Target Architecture */
#define IWBF_HDR_ARCH_I386 1

/**
 * The header is stored at the beginning of the file
 *
 * The header is immediately followed by segment_count
 * iwbf32_segment descriptors
 */
struct iwbf32_hdr {
  char magic[4]; // must be IWBF_HDR_MAGIC

  uint16_t type;
  uint16_t arch;

  /*
   * Virtual address at which execution starts
   *
   * This value is loaded into the instruction pointer
   * when transferring control to the process
   */
  uint32_t entry;

  /* Number of segments described by the segment table */
  uint32_t segment_count;
};

/*
 * Segment permissions
 *
 * These describe what the process is allowed to do with
 * the corresponding memory region
 */
#define IWBF_SEG_FLAG_READ (1u << 0)
#define IWBF_SEG_FLAG_WRITE (1u << 1)
#define IWBF_SEG_FLAG_EXEC (1u << 2)

/*
 * Describes one loadable memory region of the executable
 */
struct iwbf32_segment {
  /* Starting virtual address of the segment in the process address space */
  uint32_t vaddr;

  /*
   * Offset, in bytes, from the beginning of the IWBF file where the segment's
   * file data begins
   */
  uint32_t file_off;

  /*
   * Number of bytes belonging to this segment that are
   * actually present in the file
   *
   * If zero, the segment has no file data and
   * the entire memory region is initialized to zero
   */
  uint32_t file_size;

  /*
   * Total size of the segment in the process address space
   *
   * mem_size must be greater than or equal to file_size
   *
   * If mem_size > file_size, the remaining bytes are
   * initialized to zero
   */
  uint32_t mem_size;

  /* Memory permissions of the segment */
  uint32_t flags;
};

#endif