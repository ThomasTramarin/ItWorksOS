#ifndef KERNEL_ERROR_H
#define KERNEL_ERROR_H

/**
 * @file error.h
 *
 * @brief Kernel error definitions and error handling macros.
 */

#include <base/limits.h>
#include <base/stdint.h>

/**
 * @brief Kernel error codes
 *
 * Error codes are positive values. Functions returning errors using their
 * negative form:
 *    return -KERR_NOMEM;
 *
 * Zero indicates success
 */
enum kerror {
  KERR_OK = 0,

  KERR_INVAL,  /* argument(s) not valid */
  KERR_RANGE,  /* argument(s) out of range */
  KERR_NOMEM,  /* insufficient memory */
  KERR_NOENT,  /* resource does not exist */
  KERR_EXISTS, /* the resource already exists */
  KERR_BUSY,   /* the resource is busy */
  KERR_NOSPC,  /* insufficient resource space */
  KERR_NOSUP,  /* operation or configuration not supported */
  KERR_NODEV,  /* no device*/
};

typedef enum kerror kerror_t;

/*
 * Integer return values
 */

/**
 * @brief Propagate an error returned by an expression
 *
 * The expression must return a signed integer where negative values represent
 * errors
 *
 * If the expression returns an error, it is immediately returned from
 * the current function. Otherwise, execution continues normally
 *
 * Example:
 *    KERR_TRY(fn());
 */
#define KERR_TRY(expr)                                                         \
  do {                                                                         \
    int32_t __ret = (expr);                                                    \
    if (__ret < 0)                                                             \
      return __ret;                                                            \
  } while (0)

/**
 * @brief Check whether an integer return value represents an error
 * @param ret Signed integer return value
 */
#define KERR_IS_ERR(ret) ((ret) < 0)

/*
 * Error-valued pointers
 *
 * A range at the end of the virtual address space (the last page) is used to
 * encode negative error values as pointers.
 */

#define KERR_MAX_ERRNO 4096

/**
 * @brief Convert a negative error code into an error pointer
 * @param err Negative error code. Must not be zero because it will produce a
 *            NULL pointer
 */
#define KERR_PTR(err) ((void *)(intptr_t)(err))

/**
 * @brief Extract a negative error code from an error pointer
 */
#define KERR_PTR_ERR(ptr) ((int32_t)(intptr_t)(ptr))

/**
 * @brief Check whether a pointer represents an encoded error
 *
 */
#define KERR_PTR_IS_ERR(ptr)                                                   \
  ((uintptr_t)(ptr) >= (UINTPTR_MAX - KERR_MAX_ERRNO + 1))

/**
 * @brief Propagate an error pointer returned by an expression
 *
 * The expression must return a pointer that either represents a valid
 * object or an encoded error pointer
 *
 * If the expression returns an error pointer, it is immediately
 * returned from the current function. Otherwise, execution continues
 * normally
 *
 * Example:
 *    KERR_PTR_TRY(fn());
 */
#define KERR_PTR_TRY(expr)                                                     \
  do {                                                                         \
    void *__ret = (expr);                                                      \
    if (KERR_PTR_IS_ERR(__ret))                                                \
      return __ret;                                                            \
  } while (0)

#endif