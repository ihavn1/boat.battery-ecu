#pragma once

#include <cstdint>
#include "battery_monitor.h"

namespace sensesp {

/**
 * @brief Wire up GPIO-triggered emergency persistence before power loss
 *
 * Watches `pin` for a low signal (imminent power-supply cutoff), persists
 * both battery monitors' state, then puts the ESP32 into deep sleep.
 *
 * @param house_monitor House battery monitor to persist
 * @param starter_monitor Starter battery monitor to persist
 * @param pin GPIO pin to watch (active low)
 * @param debounce_ms Minimum time the pin must stay low before triggering
 */
void setupShutdownMonitor(BatteryMonitor& house_monitor,
                          BatteryMonitor& starter_monitor, uint8_t pin,
                          uint64_t debounce_ms);

}  // namespace sensesp
