#include <base/limits.h>
#include <base/sections.h>
#include <base/stddef.h>
#include <device/platform.h>
#include <device/resource.h>
#include <hal/interrupt.h>
#include <hal/port.h>
#include <irq/irq.h>
#include <irq/map.h>
#include <kernel/error.h>
#include <kernel/initcall.h>
#include <mm/kheap.h>
#include <time/timer_device.h>

/**
 * @brief Legacy PIT (Programmable Interval Timer) driver (Intel 8254)
 *
 * The PIT provides three independent 16-bit timer channels. All channels
 * receive the same input clock (1.193182 MHz, for compatibility reasons)  and
 * independently divide it according to to their programmed counter value.
 *
 * Channel 0 is connected to IRQ0 and is used as the system timer. Channel 1 was
 * historically used for DRAM refresh. Channel 2 is connected to the PC speaker.
 *
 */

#define I8254_CH0_OFFSET 0
#define I8254_CH1_OFFSET 1
#define I8254_CH2_OFFSET 2
#define I8254_CMD_OFFSET 3

#define I8254_BASE_FREQ 1193182ULL

#define NS_PER_SEC 1000000000ULL

/* 16761 ns */
#define I8254_MIN_DELTA_NS ((NS_PER_SEC * 20) / I8254_BASE_FREQ)

/* ~54.92 ms*/
#define I8254_MAX_DELTA_NS ((NS_PER_SEC * UINT16_MAX) / I8254_BASE_FREQ)

/**
 * @brief Command Byte
 *
 * This byte is written in the CMD port before writing frequency divisor
 * (low/high) in the selected channel port
 *
 * Bit layout:
 *  - Bits 7-6: channel (00 = channel 0)
 *  - Bits 5-4: Access mode (11 = lobyte/hibyte)
 *  - Bits 3-1: Operating mode: Mode 0 (interrupt on terminal count, 000) Mode 3
 *    (square wave, 011)
 *  - Bit 0: BCD mode (0 = binary counter , 1 = BCD counter)
 */

/**
 * @brief Private device information
 * The device's @p driver_data points to an instance of this struct
 */
struct i8254_priv {
  struct timer_device timer;
  uint32_t port_base;
  irq_t irq;
};

static int32_t i8254_timer_start_periodic(struct timer_device *timer,
                                          uint64_t period_ns) {

  if (!timer)
    return -KERR_INVAL;

  if (period_ns < timer->min_delta_ns || period_ns > timer->max_delta_ns)
    return -KERR_RANGE;

  uint64_t pit_counter = (period_ns * I8254_BASE_FREQ) / NS_PER_SEC;

  struct i8254_priv *priv = container_of(timer, struct i8254_priv, timer);

  bool interrupt_state = hal_interrupt_save_disable();

  /* Write CMD byte first: Mode 3 (square wave mode), channel 0 */
  hal_port_write8(priv->port_base + I8254_CMD_OFFSET, 0b00110110);

  /* Write lobyte and then hibyte (channel 0)*/
  hal_port_write8(priv->port_base + I8254_CH0_OFFSET,
                  (uint8_t)(pit_counter & 0xFF));
  hal_port_write8(priv->port_base + I8254_CH0_OFFSET,
                  (uint8_t)((pit_counter >> 8) & 0xFF));

  KERR_TRY(irq_enable(priv->irq));
  timer->mode = TIMER_DEVICE_MODE_PERIODIC;

  hal_interrupt_restore(interrupt_state);

  return KERR_OK;
}

static int32_t i8254_timer_start_oneshot(struct timer_device *timer,
                                         uint64_t delta_ns) {
  if (!timer)
    return -KERR_INVAL;

  if (delta_ns < timer->min_delta_ns || delta_ns > timer->max_delta_ns)
    return -KERR_RANGE;

  uint64_t pit_counter = (delta_ns * I8254_BASE_FREQ) / NS_PER_SEC;

  struct i8254_priv *priv = container_of(timer, struct i8254_priv, timer);

  bool interrupt_state = hal_interrupt_save_disable();

  /* Write CMD byte first: Mode 0 (interrupt on therminal count), channel 0 */
  hal_port_write8(priv->port_base + I8254_CMD_OFFSET, 0b00110000);

  /* Write lobyte and then hibyte (channel 0)*/
  hal_port_write8(priv->port_base + I8254_CH0_OFFSET,
                  (uint8_t)(pit_counter & 0xFF));
  hal_port_write8(priv->port_base + I8254_CH0_OFFSET,
                  (uint8_t)((pit_counter >> 8) & 0xFF));

  KERR_TRY(irq_enable(priv->irq));
  timer->mode = TIMER_DEVICE_MODE_ONESHOT;

  hal_interrupt_restore(interrupt_state);

  return KERR_OK;
}

static int32_t i8254_timer_stop(struct timer_device *timer) {
  if (!timer)
    return -KERR_INVAL;

  struct i8254_priv *priv = container_of(timer, struct i8254_priv, timer);

  bool interrupt_state = hal_interrupt_save_disable();

  priv->timer.mode = TIMER_DEVICE_MODE_DISABLED;
  KERR_TRY(irq_disable(priv->irq));

  hal_interrupt_restore(interrupt_state);

  return KERR_OK;
}

static void i8254_irq_handler(void *dev_id) {
  if (!dev_id)
    return;

  struct i8254_priv *priv = (struct i8254_priv *)dev_id;

  if (priv->timer.mode != TIMER_DEVICE_MODE_DISABLED && priv->timer.event)
    priv->timer.event(&priv->timer);

  if (priv->timer.mode == TIMER_DEVICE_MODE_ONESHOT) {
    bool interrupt_state = hal_interrupt_save_disable();

    irq_disable(priv->irq);

    priv->timer.mode = TIMER_DEVICE_MODE_DISABLED;

    hal_interrupt_restore(interrupt_state);
  }
}

static int32_t i8254_probe(struct platform_device *pdev) {

  struct i8254_priv *dev_priv = kmalloc(sizeof(struct i8254_priv));

  if (KERR_PTR_IS_ERR(dev_priv))
    return KERR_PTR_ERR(dev_priv);

  dev_priv->timer.mode = TIMER_DEVICE_MODE_DISABLED;
  dev_priv->timer.name = pdev->dev.name;
  dev_priv->timer.min_delta_ns = I8254_MIN_DELTA_NS;
  dev_priv->timer.max_delta_ns = I8254_MAX_DELTA_NS;
  dev_priv->timer.start_periodic = i8254_timer_start_periodic;
  dev_priv->timer.start_oneshot = i8254_timer_start_oneshot;
  dev_priv->timer.stop = i8254_timer_stop;
  dev_priv->timer.event = NULL;

  struct resource *port_res = resource_get(
      pdev->resources, pdev->resource_count, RESOURCE_TYPE_PORT, 0);

  struct resource *irq_res =
      resource_get(pdev->resources, pdev->resource_count, RESOURCE_TYPE_IRQ, 0);

  if (KERR_PTR_IS_ERR(port_res) || KERR_PTR_IS_ERR(irq_res)) {
    kfree(dev_priv);
    return -KERR_INVAL;
  }

  dev_priv->port_base = port_res->port.start;

  /* Resolve kernel interrupt number */
  irq_t irq = irq_map_hwirq_to_irq(irq_res->irq.map, irq_res->irq.hwirq);

  if (irq == IRQ_INVALID) {
    kfree(dev_priv);
    return -KERR_NOENT;
  }

  dev_priv->irq = irq;

  /*
   * Register the timer interrupt handler in the IRQ descriptor table.
   * The IRQ line remains disabled because the timer starts disabled.
   */
  int32_t ret = irq_request(irq, i8254_irq_handler, dev_priv);

  if (ret < 0) {
    kfree(dev_priv);
    return ret;
  }

  pdev->dev.driver_data = dev_priv;

  ret = timer_device_register(&dev_priv->timer);

  if (ret < 0) {
    irq_free(irq);
    kfree(dev_priv);
    pdev->dev.driver_data = NULL;
    return ret;
  }

  return KERR_OK;
}

static struct platform_driver i8254_driver = {
    .drv =
        {
            .name = "i8254",
        },
    .probe = i8254_probe,
};

int32_t __init i8254_driver_init(void) {
  return platform_driver_register(&i8254_driver);
}

INITCALL(INIT_DEVDRV, i8254_driver_init);