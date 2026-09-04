#include <base/stddef.h>
#include <device/bus.h>
#include <device/device.h>
#include <device/driver.h>
#include <kernel/error.h>
#include <kernel/printk.h>

/**
 * @brief Global list of registered buses
 *
 */
static struct list buses = LIST_INIT(buses);

int32_t bus_register(struct bus *bus) {
  if (!bus || !bus->match || !bus->probe)
    return -KERR_INVAL;

  list_insert_back(&buses, &bus->node);

  pr_debug("bus: registered '%s'\n", bus->name);

  return KERR_OK;
}

int32_t bus_add_device(struct bus *bus, struct device *dev) {
  if (!bus || !dev)
    return -KERR_INVAL;
  pr_debug("device: registered '%s-%d' on bus '%s'\n", dev->name, dev->id,
           bus->name);

  list_insert_back(&bus->devices, &dev->bus_node);

  struct list_node *drv_node;
  list_for_each(drv_node, &bus->drivers) {
    struct driver *drv = container_of(drv_node, struct driver, bus_node);

    if (!bus->match(dev, drv))
      continue;

    pr_debug("device: probing driver '%s' for '%s-%d'\n", drv->name, dev->name,
             dev->id);

    int32_t ret = bus->probe(dev, drv);

    if (ret == KERR_OK) {
      dev->driver = drv;

      pr_debug("device: bound '%s-%d' to driver '%s'\n", dev->name, dev->id,
               drv->name);

      return KERR_OK;
    }

    pr_debug("device: probe by driver '%s' for '%s-%d' failed: %d\n", drv->name,
             dev->name, dev->id, ret);
  }

  return KERR_OK;
}

int32_t bus_add_driver(struct bus *bus, struct driver *drv) {
  if (!bus || !drv)
    return -KERR_INVAL;

  list_insert_back(&bus->drivers, &drv->bus_node);

  pr_debug("driver: registered '%s' on bus '%s'\n", drv->name, bus->name);

  struct list_node *dev_node;
  list_for_each(dev_node, &bus->devices) {
    struct device *dev = container_of(dev_node, struct device, bus_node);

    if (dev->driver)
      continue;

    if (!bus->match(dev, drv))
      continue;

    pr_debug("device: probing driver '%s' for '%s-%d'\n", drv->name, dev->name,
             dev->id);

    int32_t ret = bus->probe(dev, drv);

    if (ret == KERR_OK) {
      dev->driver = drv;

      pr_debug("device: bound '%s-%d' to driver '%s'\n", dev->name, dev->id,
               drv->name);

      break;
    }

    pr_debug("device: probe by driver '%s' for '%s-%d' failed: %d\n", drv->name,
             dev->name, dev->id, ret);
  }

  return KERR_OK;
}