#include <kernel/error.h>
#include <time/timer_device.h>

/**
 * @brief List of all registered timer devices
 */
static struct list timer_devices = LIST_INIT(timer_devices);

/**
 * @brief Pointer to the timer device currently selected from the
 * timer_devices list
 * Currently only one timer device exists. It will be useful with multiple
 * devices and rating selection.
 */
static struct timer_device *selected = NULL;

int32_t timer_device_register(struct timer_device *timer) {
  if (!timer)
    return -KERR_INVAL;
  list_insert_back(&timer_devices, &timer->timer_node);

  selected = timer; // TODO: selection based on device rating

  return KERR_OK;
}

struct timer_device *timer_device_get_selected(void) { return selected; }