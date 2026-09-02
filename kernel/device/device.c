#include <device/bus.h>
#include <device/device.h>
#include <kernel/error.h>

int32_t device_register(struct device *dev) {
  if (!dev || !dev->bus)
    return -KERR_INVAL;

  return bus_add_device(dev->bus, dev);
}
