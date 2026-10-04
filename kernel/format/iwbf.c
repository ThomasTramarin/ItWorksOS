#include <format/iwbf.h>
#include <kernel/error.h>
#include <klib/endian.h>

int32_t iwbf_parse_hdr(const void *buf, size_t size, struct iwbf32_hdr *out) {
  if (!buf || !out)
    return -KERR_INVAL;

  if (size < IWBF32_HDR_SIZE)
    return -KERR_INVAL;

  const uint8_t *p = buf;

  out->magic = read_le32(p);
  p += 4;

  out->type = read_le16(p);
  p += 2;

  out->arch = read_le16(p);
  p += 2;

  out->entry = read_le32(p);
  p += 4;

  out->segment_count = read_le16(p);

  // Validation
  if (out->magic != IWBF_HDR_MAGIC)
    return -KERR_INVAL;

  if (out->type != IWBF_HDR_TYPE_EXEC)
    return -KERR_INVAL;

  if (out->arch != IWBF_HDR_ARCH_I386)
    return -KERR_INVAL;

  if (out->segment_count == 0)
    return -KERR_INVAL;

  return KERR_OK;
}

int32_t iwbf_parse_segment_hdr(const void *buf, size_t size,
                               struct iwbf32_segment_hdr *out) {
  if (!buf || !out)
    return -KERR_INVAL;

  if (size < IWBF32_SEGMENT_HDR_SIZE)
    return -KERR_INVAL;

  const uint8_t *p = buf;

  out->vaddr = read_le32(p);
  p += 4;

  out->file_off = read_le32(p);
  p += 4;

  out->file_size = read_le32(p);
  p += 4;

  out->mem_size = read_le32(p);
  p += 4;

  out->flags = read_le32(p);

  // Validation

  /* A segment must occupy at least as much memory
   * as its data occupies in the file */
  if (out->mem_size < out->file_size)
    return -KERR_INVAL;

  /* A segment must have at least one permission. */
  if ((out->flags &
       (IWBF_SEG_FLAG_READ | IWBF_SEG_FLAG_WRITE | IWBF_SEG_FLAG_EXEC)) == 0)
    return -KERR_INVAL;

  /* Reject unknown flags. */
  if (out->flags &
      ~(IWBF_SEG_FLAG_READ | IWBF_SEG_FLAG_WRITE | IWBF_SEG_FLAG_EXEC))
    return -KERR_INVAL;

  return KERR_OK;
}