#ifndef LOG_PRINTK_H
#define LOG_PRINTK_H
#include <base/stdarg.h>

#include <log/log.h>

int printk(enum log_level level, const char *fmt, ...);
int vprintk(enum log_level level, const char *fmt, va_list args);

#define pr_fatal(fmt, ...) printk(LOG_FATAL, fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...) printk(LOG_ERROR, fmt, ##__VA_ARGS__)
#define pr_warn(fmt, ...) printk(LOG_WARN, fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...) printk(LOG_INFO, fmt, ##__VA_ARGS__)
#define pr_debug(fmt, ...) printk(LOG_DEBUG, fmt, ##__VA_ARGS__)

#endif