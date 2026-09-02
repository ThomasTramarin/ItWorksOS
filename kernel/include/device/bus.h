#ifndef DEVICE_BUS_H
#define DEVICE_BUS_H

#include <base/stdbool.h>
#include <klib/list.h>

struct device;
struct driver;

/**
 * @brief Generic bus type structure
 *
 * A bus maintains the devices and drivers associated with it.
 * The bus is responsible for matching devices with compatible drivers and
 * initiating device probing.
 */
struct bus {
  const char *name;

  struct list drivers;
  struct list devices;

  /**
   * @brief Match a device with a compatible driver
   *
   * The match function takes a device and a driver and, based on the bus
   * matching logic, returns true if the driver is compatible with the device or
   * false otherwise.
   *
   * For example, a matching logic could compare the driver name with the device
   * name and return true if they are equal.
   *
   * This function is called when registering either a driver or a device
   */
  bool (*match)(struct device *dev, struct driver *drv);

  /**
   * @brief Probe a device with a driver
   *
   * Called after a driver has been successfully matched with a device.
   *
   * The bus may perform any bus-specific preparation required before invoking
   * the driver's probe function
   *
   * The probe function should return zero on success or a negative * error code
   * on failure.
   */
  int32_t (*probe)(struct device *dev, struct driver *drv);

  /**
   * @brief Intrusive bus node (global bus registration)
   *
   */
  struct list_node node;
};

/**
 * @brief Initialize bus
 *
 * Initializes the internal device and driver lists of the bus.
 *
 * @param bus The bus to initialize
 */
static inline void bus_init(struct bus *bus) {
  list_init(&bus->devices);
  list_init(&bus->drivers);
}

/**
 * @brief Register a bus
 *
 * Adds the bus to the global list of registered buses.
 *
 * @pre @p bus has been initialized with bus_init().
 * @pre @p bus is not already registered.
 *
 * @param bus Bus to register
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t bus_register(struct bus *bus);

/**
 * @brief Add a device to a bus
 *
 * Associates the device with the specified bus and attempts to find a
 * compatible driver for it (through bus->match()). If found, calls bus->probe()
 * (which internally invokes drv->probe()) to associate the driver to the
 * device.
 *
 * @pre @p bus is already registered
 * @pre @p dev is not already registered on a bus
 *
 * @param bus Bus to add the device to
 * @param dev Device to add
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t bus_add_device(struct bus *bus, struct device *dev);

/**
 * @brief Add a driver to a bus
 *
 * Associates the driver with the specified bus.
 * The function iterates over each device currently registered on the bus,
 * attempting to match driver with each device. If a compatible device is found,
 * the bus initiates the device probing process.
 *
 * @pre @p bus is already registered
 * @pre @p drv is not already registered on a bus
 *
 * @param bus Bus to add the driver to
 * @param drv Driver to add
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t bus_add_driver(struct bus *bus, struct driver *drv);

#endif