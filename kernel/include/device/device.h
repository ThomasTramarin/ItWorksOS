#ifndef DEVICE_DEVICE_H
#define DEVICE_DEVICE_H

#include <base/stddef.h>
#include <klib/list.h>

struct bus;
struct driver;

/**
 * @brief Generic device
 *
 * Represents a device registered in the kernel device model
 */
struct device {
  /**
   * @brief Device name
   */
  const char *name;

  /**
   * @brief Device instance identifier
   * Used to distinguish multiple instances of devices with the same name.
   */
  uint32_t id;

  /**
   * @brief Bus to which the device belongs
   */
  struct bus *bus;

  /**
   * @brief Driver currently bound to the device
   *
   * NULL if no driver is currently bound.
   */
  struct driver *driver;

  /**
   * @brief Driver private data
   *
   * Available for the bound driver to store device-specific data
   */
  void *driver_data;

  /**
   * @brief Intrusive node used to link the device into the bus device list
   */
  struct list_node bus_node;
};

/**
 * @brief Register a device in the kernel device model
 *
 * Registers the device with the bus specified by @p dev->bus
 * The bus then attempts to match the device with a compatible driver
 *
 * @pre @p dev->bus must be valid
 *
 * @param dev Device to register
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t device_register(struct device *dev);

#endif