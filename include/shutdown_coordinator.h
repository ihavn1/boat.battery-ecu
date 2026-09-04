#pragma once

#include <cstdint>
#include "battery_monitor.h"
#include "shutdown_interfaces.h"

namespace sensesp {

/**
 * @brief Persists both battery monitors and enters deep sleep on power loss
 *
 * Driven by repeated calls to sample() reflecting the current state of a
 * GPIO pin that goes low shortly before the supply voltage is cut. Debounces
 * the signal, then persists house and starter battery state before entering
 * deep sleep. Framework-free so it can be unit tested without hardware.
 */
class ShutdownCoordinator {
 public:
  /**
   * @param clock Time source used for debouncing
   * @param house_monitor House battery monitor to persist
   * @param starter_monitor Starter battery monitor to persist
   * @param sleep_controller Controller invoked once persistence succeeds
   * @param debounce_ms Minimum time the pin must stay low before triggering
   */
  ShutdownCoordinator(const IClock& clock, BatteryMonitor& house_monitor,
                      BatteryMonitor& starter_monitor,
                      ISleepController& sleep_controller,
                      uint64_t debounce_ms)
      : clock_(clock),
        house_monitor_(house_monitor),
        starter_monitor_(starter_monitor),
        sleep_controller_(sleep_controller),
        debounce_ms_(debounce_ms),
        low_since_ms_(0),
        debounce_started_(false),
        complete_(false) {}

  /**
   * @brief Feed the current shutdown-pin state into the coordinator
   * @param shutdown_active true if the shutdown pin currently reads low
   */
  void sample(bool shutdown_active) {
    if (complete_) {
      return;
    }

    if (!shutdown_active) {
      debounce_started_ = false;
      return;
    }

    const uint64_t now_ms = clock_.now_ms();
    if (!debounce_started_) {
      debounce_started_ = true;
      low_since_ms_ = now_ms;
      if (debounce_ms_ > 0) {
        return;
      }
    }

    if (now_ms < low_since_ms_ || now_ms - low_since_ms_ < debounce_ms_) {
      return;
    }

    // Require both saves to succeed so a partial write never leaves one
    // battery's persisted state stale after power is cut.
    if (!house_monitor_.save_state() || !starter_monitor_.save_state()) {
      return;
    }

    complete_ = true;
    sleep_controller_.enter_deep_sleep();
  }

  /**
   * @brief Whether persistence has completed and deep sleep was requested
   */
  bool is_complete() const { return complete_; }

 private:
  const IClock& clock_;
  BatteryMonitor& house_monitor_;
  BatteryMonitor& starter_monitor_;
  ISleepController& sleep_controller_;
  uint64_t debounce_ms_;
  uint64_t low_since_ms_;
  bool debounce_started_;
  bool complete_;
};

}  // namespace sensesp
