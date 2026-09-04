#ifndef TIME_TIMER_DEVICE_H
#define TIME_TIMER_DEVICE_H

#include <base/stdint.h>
#include <klib/list.h>

enum timer_device_mode {
  /**
   * @brief Timer disabled
   *
   * In this mode, the timer will not invoke the event() callback.
   */
  TIMER_DEVICE_MODE_DISABLED,

  /**
   * @brief Periodic event mode
   *
   * The timer continuously triggers interrupts at a fixed interval specified by
   * start_periodic(), invoking event() periodically.
   */
  TIMER_DEVICE_MODE_PERIODIC,

  /**
   * @brief One-shot event mode
   * The timer triggers a single interrupt after a specified delay, then stops.
   */
  TIMER_DEVICE_MODE_ONESHOT
};

/**
 * @brief Generic interface for timer devices
 *
 * This abstraction is not part of the kernel device model.
 * Timer drivers register their hardware-specific implementation through this
 * interface, allowing the time subsystem to operate on timers without depending
 * on hardware-specific details.
 */
struct timer_device {
  const char *name;

  /**
   * @brief The current timer mode
   * This variable must not be edited directly because modifying mode requires
   * some hardware-specific operations. So it is read-only from the
   * caller view.
   */
  enum timer_device_mode mode;

  /**
   * @brief Minimum supported timer interval in nanoseconds
   *
   * The caller is responible for choosing an interval that is greater than or
   * equal this value. If the caller specifies a lower value, the function
   * returns -KERR_RANGE.
   */
  uint64_t min_delta_ns;

  /**
   * @brief Maximum supported timer interval in nanoseconds
   *
   * The caller is responible for choosing an interval that is less than or
   * equal this value. If the caller specifies an higher value, the function
   * returns -KERR_RANGE.
   */
  uint64_t max_delta_ns;

  /**
   * @brief Program the timer for periodic operation
   *
   * Configures the hardware to generate interrupts every @p period_ns
   * nanoseconds. Each interrupt invokes the event() callback, so the caller
   * have to set it before calling this function.
   *
   * @param timer Pointer to this timer_device instance
   * @param period_ns Target period in nanoseconds
   * @return KERR_OK on success, or negative error code
   */
  int32_t (*start_periodic)(struct timer_device *timer, uint64_t period_ns);

  /**
   * @brief Program the timer for one-shot operation
   *
   * Configures the hardware to generate a single interrupt after
   * @p delta_ns nanoseconds. The event() callback is invoked when the
   * timer expires, after which the timer enters disabled mode.
   *
   * @param timer Pointer to this timer_device instance.
   * @param delta_ns Delay in nanoseconds before the timer expires.
   * @return KERR_OK on success, or negative error code.
   */
  int32_t (*start_oneshot)(struct timer_device *timer, uint64_t delta_ns);

  /**
   * @brief Stop the timer
   *
   * After invoking this function, the timer passes to disabled mode and will
   * not invoke the event() callback.
   *
   * @param timer Pointer to this timer_device instance.
   * @return KERR_OK on success, or negative error code
   */
  int32_t (*stop)(struct timer_device *timer);

  /**
   * @brief Callback invoked upon timer expiration
   *
   * This field is not set by the driver.
   *
   * @param timer Pointer to the timer_device instance that triggered the IRQ.
   */
  void (*event)(struct timer_device *timer);

  struct list_node timer_node;
};

/**
 * @brief Register a timer device
 *
 * Adds the timer device to the list of registered timer devices.
 * The newly registered device becomes the selected timer.
 * @param timer Pointer to the timer device to register
 * @return KERR_OK on success, or a negative error code
 */
int32_t timer_device_register(struct timer_device *timer);

/**
 * @brief Get the current selected timer
 *
 * @return Pointer to the selected timer or NULL if no timer device has been
 *         registered yet
 */
struct timer_device *timer_device_get_selected(void);

#endif