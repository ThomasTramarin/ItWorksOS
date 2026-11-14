#ifndef LOG_BACKEND_H
#define LOG_BACKEND_H

#include <base/stdint.h>
#include <klib/list.h>
#include <log/log.h>

/**
 * @brief Log output backend
 *
 * A backend receives log records and forwards them to
 * an output destination, such as a console or debug port
 */
struct log_backend {
  const char *name;

  /**
   * @brief Writes a log record to this backend
   *
   * @param record Log record to write
   * @return KERR_OK on success, negative kernel error code on failure
   */
  int32_t (*write)(const struct log_record *record);

  struct list_node log_backend_node;
};

/**
 * @brief Registers a log backend
 *
 * @param backend Backend to register
 * @return KERR_OK on success, negative kernel error code on failure
 */
int32_t log_backend_register(struct log_backend *backend);

/**
 * @brief Dispatches a log record to registered backends
 *
 * @param record Record to dispatch
 * @return KERR_OK on success, negative kernel error code on failure
 */
int32_t log_backend_dispatch(const struct log_record *record);

#endif