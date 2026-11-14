#ifndef LOG_LOG_H
#define LOG_LOG_H

#include <base/stddef.h>

/**
 * @brief Severity levels for kernel log messages
 */
enum log_level {
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
  LOG_FATAL,
};

/**
 * @brief The record in the ringbuffer is stored as:
 *
 * +-------------------------------+
 * | header                        |
 * +-------------------------------+
 * | header.length bytes (message) |
 * +-------------------------------+
 *
 * The log record may not be contiguous in memory
 * due to the circular buffer
 */
struct log_record_header {
  uint32_t seq;
  enum log_level level;
  size_t length;
};

/**
 * @brief Logical log record
 *
 * This structure is passed to log_backend->write()
 * Unlike the representation stored in the ring buffer,
 * the message is contiguous in memory
 */
struct log_record {
  struct log_record_header hdr;
  const char *message;
};

#endif