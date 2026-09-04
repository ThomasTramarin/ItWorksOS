#include <base/sections.h>
#include <device/platform.h>
#include <device/resource.h>
#include <drivers/irqchip/i8259.h>
#include <irq/irq.h>
#include <kernel/initcall.h>

extern struct irq_map i8259_map;

static struct resource i8254_0_resources[] = {
    RESOURCE_PORT(0x40, 0x43),
    RESOURCE_IRQ(0, &i8259_map), /* PIC hwirq */
};

static struct platform_device i8254_0_device = {
    .dev =
        {
            .name = "i8254",
            .id = 0,
        },
    .resources = i8254_0_resources,
    .resource_count = sizeof(i8254_0_resources) / sizeof(i8254_0_resources[0]),
};

static int32_t __init i8254_device_init(void) {
  return platform_device_register(&i8254_0_device);
}

INITCALL(INIT_DEVDRV, i8254_device_init);