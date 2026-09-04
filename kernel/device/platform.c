#include <base/sections.h>
#include <device/bus.h>
#include <device/platform.h>
#include <kernel/error.h>
#include <klib/string.h>

static bool platform_match(struct device *dev, struct driver *drv) {
  if (!dev || !drv || !dev->name || !drv->name)
    return false;

  return (strcmp(dev->name, drv->name) == 0);
}

static int32_t platform_probe(struct device *dev, struct driver *drv) {
  struct platform_device *pdev = container_of(dev, struct platform_device, dev);
  struct platform_driver *pdrv = container_of(drv, struct platform_driver, drv);

  if (!pdrv->probe)
    return -KERR_INVAL;

  return pdrv->probe(pdev);
}

/**
 * @brief Platform bus
 *
 * Virtual bus to manage devices that are directly provided by the system and do
 * not belong to a discoverable bus.
 */
struct bus platform_bus = {
    .name = "platform", .match = platform_match, .probe = platform_probe};

int32_t __init platform_init(void) {

  bus_init(&platform_bus);

  return bus_register(&platform_bus);
}

int32_t platform_driver_register(struct platform_driver *pdrv) {
  if (!pdrv)
    return -KERR_INVAL;

  pdrv->drv.bus = &platform_bus;

  return driver_register(&pdrv->drv);
}

int32_t platform_device_register(struct platform_device *pdev) {
  if (!pdev)
    return -KERR_INVAL;

  pdev->dev.bus = &platform_bus;

  return device_register(&pdev->dev);
}