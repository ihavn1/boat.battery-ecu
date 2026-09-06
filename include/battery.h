#pragma once

#include <Arduino.h>
#include "battery_config.h"
#include "ah_calculator.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#endif

namespace sensesp {

struct BatteryStateSnapshot {
  double ah;
  float marked_capacity_ah;
  float current_capacity_ah;
  float charge_efficiency;
  float discharge_efficiency;
};

/**
 * @brief Battery domain model encapsulating state and behavior
 * 
 * Represents a single battery with its state (voltage, current, Ah, SOC)
 * and behavior (charging, discharging, SOC calculation). Follows the
 * Single Responsibility Principle by focusing only on battery domain logic,
 * delegating persistence to storage providers and sensor reading to sensors.
 * 
 * This class is the core domain model, free from infrastructure concerns
 * like Signal K, sensors, or persistence mechanisms.
 * 
 * Note: SOC is calculated as percentage (0-100%) for internal use.
 * Signal K output converts to ratio (0-1) by dividing by 100.
 */
class Battery {
 public:
  /**
   * @brief Construct a battery with configuration
   * @param config Battery configuration (name, capacity, paths)
   */
  explicit Battery(const BatteryConfig& config)
      : config_(config),
        calculator_(config.initial_ah(), config.marked_capacity_ah()),
        voltage_(0.0f),
        current_(0.0f),
        power_(0.0f),
        temperature_(20.0f)
#if defined(ARDUINO_ARCH_ESP32)
        , state_mutex_(xSemaphoreCreateMutex())
#endif
  {}

  ~Battery() {
#if defined(ARDUINO_ARCH_ESP32)
    vSemaphoreDelete(state_mutex_);
#endif
  }

  // Configuration accessors
  const BatteryConfig& config() const { return config_; }
  const char* name() const { return config_.name(); }
  const char* chip_name() const { return config_.chip_name(); }

  // State accessors
  float voltage() const { Lock lock(*this); return voltage_; }
  float current() const { Lock lock(*this); return current_; }
  float power() const { Lock lock(*this); return power_; }
  float temperature() const { Lock lock(*this); return temperature_; }
  
  double ah() const { Lock lock(*this); return calculator_.get_ah(); }
  // Returns SOC as percentage (0-100%). For Signal K, divide by 100 to get ratio (0-1)
  float soc() const { Lock lock(*this); return calculator_.calculate_soc(); }
  
  float marked_capacity_ah() const { Lock lock(*this); return calculator_.get_marked_capacity_ah(); }
  float current_capacity_ah() const { Lock lock(*this); return calculator_.get_current_capacity_ah(); }
  
  float charge_efficiency() const { Lock lock(*this); return calculator_.get_charge_efficiency(); }
  float discharge_efficiency() const { Lock lock(*this); return calculator_.get_discharge_efficiency(); }

  BatteryStateSnapshot snapshot() const {
    Lock lock(*this);
    return {calculator_.get_ah(), calculator_.get_marked_capacity_ah(),
            calculator_.get_current_capacity_ah(), calculator_.get_charge_efficiency(),
            calculator_.get_discharge_efficiency()};
  }

  // State mutators
  void set_voltage(float v) { Lock lock(*this); voltage_ = v; }
  void set_current(float a) { Lock lock(*this); current_ = a; }
  void set_power(float w) { Lock lock(*this); power_ = w; }
  void set_temperature(float c) { Lock lock(*this); temperature_ = c; }
  
  void set_ah(double ah) { Lock lock(*this); calculator_.set_ah(ah); }
  void set_marked_capacity_ah(float capacity_ah) { 
    Lock lock(*this);
    calculator_.set_marked_capacity_ah(capacity_ah); 
  }
  void set_current_capacity_ah(float capacity_ah) { 
    Lock lock(*this);
    calculator_.set_current_capacity_ah(capacity_ah); 
  }
  
  void set_charge_efficiency(float pct) { 
    Lock lock(*this);
    calculator_.set_charge_efficiency(pct); 
  }
  void set_discharge_efficiency(float pct) { 
    Lock lock(*this);
    calculator_.set_discharge_efficiency(pct); 
  }

  // Domain behavior
  
  /**
   * @brief Update battery state with sensor readings
   * @param voltage_v Voltage in volts
   * @param current_a Current in amperes (positive = charging, negative = discharging)
   * @param power_w Power in watts
   */
  void update_readings(float voltage_v, float current_a, float power_w) {
    Lock lock(*this);
    voltage_ = voltage_v;
    current_ = current_a;
    power_ = power_w;
  }

  /**
   * @brief Integrate current over time to update Ah
   * @param current_a Current in amperes
   * @param dt_ms Time delta in milliseconds
   */
  void integrate_current(float current_a, unsigned long dt_ms) {
    Lock lock(*this);
    calculator_.integrate_current(current_a, dt_ms);
  }

  /**
   * @brief Check if battery is charging
   * @return true if current > 0
   */
  bool is_charging() const {
    Lock lock(*this);
    return current_ > 0.0f;
  }

  /**
   * @brief Check if battery is discharging
   * @return true if current < 0
   */
  bool is_discharging() const {
    Lock lock(*this);
    return current_ < 0.0f;
  }

  /**
   * @brief Check if battery state has changed significantly
   * @param previous_ah Previous Ah value to compare against
   * @param threshold Threshold for significant change (default 0.5 Ah)
   * @return true if change is significant
   */
  bool has_ah_changed_significantly(double previous_ah, double threshold = 0.5) const {
    Lock lock(*this);
    return calculator_.has_changed_significantly(previous_ah, threshold);
  }

  /**
   * @brief Check if battery is fully charged
   * @return true if SOC >= 99%
   */
  bool is_fully_charged() const {
    return soc() >= 99.0f;
  }

  /**
   * @brief Check if battery is empty
   * @return true if SOC <= 1%
   */
  bool is_empty() const {
    return soc() <= 1.0f;
  }

  /**
   * @brief Check if battery is critically low
   * @return true if SOC <= 20%
   */
  bool is_critically_low() const {
    return soc() <= 20.0f;
  }

  /**
   * @brief Get battery health percentage
   * @return Health percentage (current capacity / marked capacity * 100)
   */
  float health_percentage() const {
    float marked = marked_capacity_ah();
    if (marked <= 0.0f) return 100.0f;
    return (current_capacity_ah() / marked) * 100.0f;
  }

 private:
  class Lock {
   public:
    explicit Lock(const Battery& battery) : battery_(battery) {
#if defined(ARDUINO_ARCH_ESP32)
      xSemaphoreTake(battery_.state_mutex_, portMAX_DELAY);
#endif
    }
    ~Lock() {
#if defined(ARDUINO_ARCH_ESP32)
      xSemaphoreGive(battery_.state_mutex_);
#endif
    }
   private:
    const Battery& battery_;
  };

  BatteryConfig config_;
  AmpHourCalculator calculator_;
  
  // Sensor readings
  float voltage_;      // Volts
  float current_;      // Amperes (positive = charging, negative = discharging)
  float power_;        // Watts
  float temperature_;  // Celsius
#if defined(ARDUINO_ARCH_ESP32)
  SemaphoreHandle_t state_mutex_;
#endif
};

}  // namespace sensesp
