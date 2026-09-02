#ifndef DEVICE_DRIVER_H
#define DEVICE_DRIVER_H

#include <klib/list.h>

struct bus;
struct device;

struct driver {
  /**
   * @brief Driver name
   */
  const char *name;

  /**
   * @brief Bus to which the driver belongs
   */
  struct bus *bus;

  /**
   * @brief Probe a device
   *
   * Called by the bus when the driver matches a compatible device.
   *
   * @param dev Device to probe
   * @return KERR_OK on success or a negative kernel error code
   */
  int32_t (*probe)(struct device *dev);

  /**
   * @brief Intrusive node used to link the driver into the bus driver list
   */
  struct list_node bus_node;
};

/**
 * @brief Register a driver in the kernel device model
 *
 * Registers the driver with the bus specified by @p drv->bus.
 * The bus then attempts to match the driver with compatible devices and probe
 * them.
 *
 * @pre @p drv->bus must be valid
 *
 * @param drv Driver to register
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t driver_register(struct driver *drv);

#endif