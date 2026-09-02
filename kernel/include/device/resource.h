#ifndef DEVICE_RESOURCE_H
#define DEVICE_RESOURCE_H

#include <base/stddef.h>
#include <base/stdint.h>
#include <irq/irq.h>

enum resource_type { RESOURCE_TYPE_PORT, RESOURCE_TYPE_IRQ };

/**
 * @brief Device hardware resource
 *
 * Describes a hardware resource associated with a device, such as an
 * I/O port range or an interrupt.
 */
struct resource {
  enum resource_type type;

  union {
    /**
     * @brief I/O port range
     *
     * Both start and end are inclusive: [start, end].
     */
    struct {
      uint32_t start;
      uint32_t end;
    } port;
    /**
     * @brief Hardware interrupt
     *
     * Identifies an interrupt by its hardware IRQ number within an
     * interrupt mapping.
     */
    struct {
      struct irq_map *map;
      hwirq_t hwirq;
    } irq;
  };
};

#define RESOURCE_PORT(s, e)                                                    \
  {                                                                            \
    .type = RESOURCE_TYPE_PORT, .port = {                                      \
      .start = (s),                                                            \
      .end = (e),                                                              \
    }                                                                          \
  }

#define RESOURCE_IRQ(n, m)                                                     \
  {                                                                            \
    .type = RESOURCE_TYPE_IRQ, .irq = {.hwirq = (n), .map = (m) }              \
  }

/**
 * @brief Get a resource of a specific type from a resource array
 *
 * Searches the resource array for the @p index-th resource matching the
 * specified resourec type.
 *
 * @param resources Array of resources
 * @param count Number of resources in the array
 * @param type Resource type to search for
 * @param index Zero-based index among resources of the specified type
 * @return Pointer to the requested resource on success, or an encoded negative
 * kernel error code on failure
 */
struct resource *resource_get(struct resource *resources, size_t count,
                              enum resource_type type, size_t index);

#endif