#include "shutdown_helper.h"

#include <esp_sleep.h>
#include <esp_timer.h>
#include "shutdown_coordinator.h"
#include "shutdown_monitor.h"

namespace sensesp {

namespace {

class EspClock : public IClock {
 public:
  uint64_t now_ms() const override {
    return static_cast<uint64_t>(esp_timer_get_time()) / 1000ULL;
  }
};

class EspDeepSleepController : public ISleepController {
 public:
  void enter_deep_sleep() override { esp_deep_sleep_start(); }
};

}  // namespace

void setupShutdownMonitor(BatteryMonitor& house_monitor,
                          BatteryMonitor& starter_monitor, uint8_t pin,
                          uint64_t debounce_ms) {
  auto* clock = new EspClock();
  auto* sleep_controller = new EspDeepSleepController();
  auto* coordinator = new ShutdownCoordinator(
      *clock, house_monitor, starter_monitor, *sleep_controller, debounce_ms);
  auto* monitor = new ShutdownMonitor(pin, *coordinator);
  monitor->begin();
}

}  // namespace sensesp
