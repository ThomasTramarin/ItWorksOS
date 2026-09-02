#ifndef DEVICE_PLATFORM_H
#define DEVICE_PLATFORM_H

#include <device/device.h>
#include <device/driver.h>

/**
 * @brief Platform device (extends device)
 *
 * Represents a device connected to the platform bus.
 */
struct platform_device {
  struct device dev;

  /**
   * @brief Device resources
   *
   * Array containing the hardware resources assigned to the device.
   */
  struct resource *resources;

  /**
   * @brief Number of resources
   *
   * Number of elements contained in the @p resources array.
   */
  size_t resource_count;
};

/**
 * @brief @brief Platform driver (extends driver)
 *
 * Represents a driver connected to the platform bus.
 */
struct platform_driver {
  struct driver drv;

  /**
   * @brief Probe a platform device (extends device->probe())
   *
   * Called when the platform bus finds a compatible device for this driver.
   *
   * @param pdev Platform device to probe
   * @return KERR_OK on success or a negative kernel error code
   */
  int32_t (*probe)(struct platform_device *pdev);
};

/**
 * @brief Initialize platform bus
 *
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t platform_init(void);

/**
 * @brief Register a platform driver
 *
 * Associates the driver with the platform bus and registers it with the generic
 * device model.
 *
 * @param pdrv Platform driver to register
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t platform_driver_register(struct platform_driver *pdrv);

/**
 * @brief Register a platform device
 *
 * Associates the device with the platform bus and registers it with the generic
 * device model.
 *
 * @param pdev Platform device
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t platform_device_register(struct platform_device *pdev);

#endif