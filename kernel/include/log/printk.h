#ifndef LOG_PRINTK_H
#define LOG_PRINTK_H
#include <base/stdarg.h>

#define PRINTK_FATAL "[FATAL] "
#define PRINTK_ERR "[ERR]   "
#define PRINTK_WARN "[WARN]  "
#define PRINTK_INFO "[INFO]  "
#define PRINTK_DEBUG "[DEBUG] "

int printk(const char *fmt, ...);
int vprintk(const char *fmt, va_list args);

#define pr_fatal(fmt, ...) printk(PRINTK_FATAL fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...) printk(PRINTK_ERR fmt, ##__VA_ARGS__)
#define pr_warn(fmt, ...) printk(PRINTK_WARN fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...) printk(PRINTK_INFO fmt, ##__VA_ARGS__)
#define pr_debug(fmt, ...) printk(PRINTK_DEBUG fmt, ##__VA_ARGS__)

#endif