#ifndef KLIB_ENDIAN_H
#define KLIB_ENDIAN_H

#include <base/stdint.h>

static inline uint16_t read_le16(const uint8_t *buf) {
  return ((uint16_t)buf[0]) | ((uint16_t)buf[1] << 8);
}

static inline uint32_t read_le32(const uint8_t *buf) {
  return ((uint32_t)buf[0]) | ((uint32_t)buf[1] << 8) |
         ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
}

static inline uint64_t read_le64(const uint8_t *buf) {
  return ((uint64_t)buf[0]) | ((uint64_t)buf[1] << 8) |
         ((uint64_t)buf[2] << 16) | ((uint64_t)buf[3] << 24) |
         ((uint64_t)buf[4] << 32) | ((uint64_t)buf[5] << 40) |
         ((uint64_t)buf[6] << 48) | ((uint64_t)buf[7] << 56);
}

static inline void write_le16(uint8_t *buf, uint16_t value) {
  buf[0] = (uint8_t)value;
  buf[1] = (uint8_t)(value >> 8);
}

static inline void write_le32(uint8_t *buf, uint32_t value) {
  buf[0] = (uint8_t)value;
  buf[1] = (uint8_t)(value >> 8);
  buf[2] = (uint8_t)(value >> 16);
  buf[3] = (uint8_t)(value >> 24);
}

static inline void write_le64(uint8_t *buf, uint64_t value) {
  buf[0] = (uint8_t)value;
  buf[1] = (uint8_t)(value >> 8);
  buf[2] = (uint8_t)(value >> 16);
  buf[3] = (uint8_t)(value >> 24);
  buf[4] = (uint8_t)(value >> 32);
  buf[5] = (uint8_t)(value >> 40);
  buf[6] = (uint8_t)(value >> 48);
  buf[7] = (uint8_t)(value >> 56);
}

#endif