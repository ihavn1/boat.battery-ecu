#pragma once

#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "shutdown_coordinator.h"

namespace sensesp {

/**
 * @brief Watches a GPIO pin for a low signal warning of imminent power loss
 *
 * A falling-edge interrupt wakes a dedicated high-priority FreeRTOS task,
 * which drives ShutdownCoordinator::sample() in a tight loop until
 * persistence completes or the pin returns high.
 */
class ShutdownMonitor {
 public:
  ShutdownMonitor(uint8_t pin, ShutdownCoordinator& coordinator);

  void begin();

 private:
  static void IRAM_ATTR interrupt_entry(void* context);
  static void task_entry(void* context);
  void run();

  uint8_t pin_;
  ShutdownCoordinator& coordinator_;
  TaskHandle_t task_handle_;
};

}  // namespace sensesp
