# Devices

## Overview

In IWOS, a **device** represents a hardware component managed by the kernel, such as a timer, keyboard controller, or VGA device.

The device subsystem provides a generic interface for representing devices and connecting them to the drivers that control them.

A device belongs to a **bus** and may have a **driver** bound to it.

The current implementation provides a single virtual bus called `platform_bus`.

For initialization, device and driver registration can be triggered by the [initcall system](./initcalls.md).

## Bus

A **bus** manages a set of devices and drivers that can be matched with each other.

A bus provides:

- a list of registered devices
- a list of registered drivers
- a `match()` function that determines whether a driver supports a device
- a `probe()` function that starts device initialization

The bus is responsible for matching devices with drivers and starting the probing process.

### Platform bus

The current implementation provides `platform_bus`.

The platform bus is a virtual bus for devices that are provided directly by the system and do not belong to a discoverable hardware bus such as PCI or USB.

Platform devices and drivers are matched by name.

## Devices

A device is represented by `struct device`.

A device contains:

- a name identifying the device type
- an instance ID used when multiple devices have the same name
- the bus the device belongs to
- the driver currently bound to the device
- optional driver-specific data

A device is registered with `device_register()`.

The device must have a bus assigned before registration.

### Device registration

When `device_register()` is called, the device is passed to its bus through `bus_add_device()`.

The bus then:

- adds the device to its list of registered devices
- checks the registered drivers using `match()`
- probes matching drivers
- binds the first driver whose probe succeeds

If no driver matches, or all matching drivers fail to probe, the device remains registered without a driver.

For platform devices, `platform_device_register()` automatically assigns `platform_bus` before registering the device.

## Drivers

A driver is represented by `struct driver`.

A driver contains:

- a name identifying the device type it supports
- the bus it belongs to
- a `probe()` function used to initialize a matched device

A driver is registered with `driver_register()`.

The driver must have a bus assigned before registration.

### Driver registration

When `driver_register()` is called, the driver is passed to its bus through `bus_add_driver()`.

The bus then:

- adds the driver to its list of registered drivers
- checks existing devices that are not already bound
- uses `match()` to find compatible devices
- probes matching devices
- binds the driver when probing succeeds

This means that devices and drivers can be registered in either order.

For example, a device can be registered before its driver exists. When the driver is later registered, the bus checks the existing device and attempts to bind the driver.

Similarly, a driver can be registered before its device exists. When the device is later registered, the bus checks the existing driver.

For platform drivers, `platform_driver_register()` automatically assigns `platform_bus` before registering the driver.

## Matching and probing

The bus uses `match()` to determine whether a driver is compatible with a device.

If `match()` returns `true`, the bus calls its `probe()` function.

The bus-specific `probe()` function performs any bus-specific handling and then calls the driver's `probe()` function.

If the driver's `probe()` function succeeds, the driver is bound to the device.

A failed probe does not bind the driver, so the bus can try another matching driver.

The current platform bus uses the device and driver names for matching.

## Resources

A device may have hardware resources required by its driver.

Resources are represented by `struct resource`.

The current resource types are:

- **I/O port**: a range of hardware I/O ports
- **IRQ**: a hardware interrupt associated with an IRQ mapping
- **MMIO**: a physical memory range used for memory-mapped I/O

A platform device stores its resources in an array.

A driver can retrieve a resource by type and index using `resource_get()`.

Resources describe the hardware resources assigned to a device. The driver uses them when initializing and operating the hardware.

## Platform devices and drivers

Platform devices and drivers extend the generic device and driver structures.

A `platform_device` contains:

- a generic `struct device`
- an array of hardware resources
- the number of resources in the array

A `platform_driver` contains:

- a generic `struct driver`
- a platform-specific `probe()` function

Their registration functions automatically associate them with `platform_bus`.

The platform bus then performs the normal registration, matching, and probing process.

## Initialization

Device and driver registration can be performed from an initcall.

For example, a driver can register itself during the `devdrv` initialization stage:

```c
static int32_t __init timer_driver_init(void)
{
    return platform_driver_register(&timer_driver);
}

INITCALL(INIT_DEVDRV, timer_driver_init);
```

When the kernel invokes the `devdrv` initcalls, the registration function adds the driver to the platform bus. The bus can then match it with already registered devices and probe them.

See [Initcalls](./initcalls.md) for the initialization mechanism.

The device subsystem is responsible for **registering, matching, and probing** devices and drivers, while the initcall system determines **when** registration functions are executed.
