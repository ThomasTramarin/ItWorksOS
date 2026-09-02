#include <device/bus.h>
#include <device/driver.h>
#include <kernel/error.h>

int32_t driver_register(struct driver *drv) {
  if (!drv || !drv->bus)
    return -KERR_INVAL;

  return bus_add_driver(drv->bus, drv);
}
