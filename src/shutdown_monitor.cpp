#include "shutdown_monitor.h"

#include <Arduino.h>
#include <esp_wifi.h>

namespace sensesp {

ShutdownMonitor::ShutdownMonitor(uint8_t pin, ShutdownCoordinator& coordinator)
    : pin_(pin), coordinator_(coordinator), task_handle_(nullptr) {}

void ShutdownMonitor::begin() {
  // No internal pull-up: the signal level is driven by an external voltage
  // divider, and an internal pull-up would load and skew its output.
  pinMode(pin_, INPUT);
  xTaskCreatePinnedToCore(task_entry, "battery-shutdown", 3072, this,
                          configMAX_PRIORITIES - 2, &task_handle_,
                          tskNO_AFFINITY);
  attachInterruptArg(pin_, interrupt_entry, this, FALLING);
  if (digitalRead(pin_) == LOW) {
    xTaskNotifyGive(task_handle_);
  }
}

void IRAM_ATTR ShutdownMonitor::interrupt_entry(void* context) {
  auto* monitor = static_cast<ShutdownMonitor*>(context);
  BaseType_t higher_priority_task_woken = pdFALSE;
  vTaskNotifyGiveFromISR(monitor->task_handle_, &higher_priority_task_woken);
  if (higher_priority_task_woken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}

void ShutdownMonitor::task_entry(void* context) {
  static_cast<ShutdownMonitor*>(context)->run();
}

void ShutdownMonitor::run() {
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if (digitalRead(pin_) != LOW) {
      continue;
    }

    // Stop the radio first so the CPU isn't competing with WiFi tasks
    // during the brief window before power is cut.
    esp_wifi_stop();
    do {
      coordinator_.sample(true);
      if (!coordinator_.is_complete()) {
        vTaskDelay(pdMS_TO_TICKS(1));
      }
    } while (digitalRead(pin_) == LOW && !coordinator_.is_complete());

    coordinator_.sample(false);
  }
}

}  // namespace sensesp
