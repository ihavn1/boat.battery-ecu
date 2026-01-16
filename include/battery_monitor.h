#pragma once

#include <Arduino.h>
#include "battery.h"
#include "sensors/i_sensor.h"
#include "storage/i_storage_provider.h"

namespace sensesp {

/**
 * @brief Orchestrates battery monitoring activities
 * 
 * Coordinates sensor reading, current integration, state persistence,
 * and state updates for a Battery. Follows Single Responsibility Principle
 * by focusing only on orchestration logic, delegating actual domain
 * behavior to Battery and infrastructure to sensors/storage.
 * 
 * This is the "application service" layer between infrastructure
 * (sensors, storage) and domain model (Battery).
 */
class BatteryMonitor {
 public:
  /**
   * @brief Construct battery monitor
   * @param battery Battery domain object to monitor
   * @param sensor Sensor for reading voltage/current/power
   * @param storage Storage provider for persisting state
   */
  BatteryMonitor(Battery& battery, ISensor& sensor, IStorageProvider& storage)
      : battery_(battery),
        sensor_(sensor),
        storage_(storage),
        last_integration_ms_(0),
        last_persist_ms_(0),
        last_persisted_ah_(battery.ah()) {
    
    // Load persisted state
    load_state();
  }

  /**
   * @brief Initialize sensor and prepare for monitoring
   * @return true if initialization successful
   */
  bool begin() {
    return sensor_.begin();
  }

  /**
   * @brief Update battery state from sensor readings
   * Should be called at regular intervals (e.g., 1Hz)
   */
  void update_readings() {
    float voltage = sensor_.getBusVoltage();
    float current = sensor_.getCurrent();
    float power = sensor_.getPower();
    
    battery_.update_readings(voltage, current, power);
  }

  /**
   * @brief Integrate current to update Ah
   * Should be called at regular intervals (e.g., 1Hz or faster)
   */
  void integrate() {
    unsigned long now = millis();
    if (last_integration_ms_ == 0) {
      last_integration_ms_ = now;
      return;
    }
    
    unsigned long dt_ms = now - last_integration_ms_;
    last_integration_ms_ = now;
    
    battery_.integrate_current(battery_.current(), dt_ms);
  }

  /**
   * @brief Persist battery state if changed significantly
   * Should be called periodically (e.g., every 5 seconds)
   * 
   * @param force Force persistence regardless of change threshold
   * @param min_interval_ms Minimum time between persists (default 10 seconds)
   * @param change_threshold Ah change threshold for persistence (default 0.5 Ah)
   */
  void maybe_persist(bool force = false, unsigned long min_interval_ms = 10000, 
                     double change_threshold = 0.5) {
    unsigned long now = millis();
    
    if (!force) {
      // Check minimum interval
      if (now - last_persist_ms_ < min_interval_ms) {
        return;
      }
      
      // Check if changed significantly
      if (!battery_.has_ah_changed_significantly(last_persisted_ah_, change_threshold)) {
        return;
      }
    }
    
    // Persist state
    if (save_state()) {
      last_persisted_ah_ = battery_.ah();
      last_persist_ms_ = now;
    }
  }

  /**
   * @brief Save battery state to storage
   * @return true if successful
   */
  bool save_state() {
    String key_prefix = String(battery_.chip_name());
    
    if (!storage_.begin("battcfg", false)) {
      return false;
    }
    
    storage_.putFloat((key_prefix + "_ah").c_str(), (float)battery_.ah());
    storage_.putFloat((key_prefix + "_marked").c_str(), battery_.marked_capacity_ah());
    storage_.putFloat((key_prefix + "_current").c_str(), battery_.current_capacity_ah());
    storage_.putFloat((key_prefix + "_charge").c_str(), battery_.charge_efficiency());
    storage_.putFloat((key_prefix + "_discharge").c_str(), battery_.discharge_efficiency());
    
    storage_.end();
    return true;
  }

  /**
   * @brief Load battery state from storage
   * @return true if state was loaded
   */
  bool load_state() {
    String key_prefix = String(battery_.chip_name());
    
    if (!storage_.begin("battcfg", true)) {
      return false;
    }
    
    bool loaded = false;
    
    if (storage_.isKey((key_prefix + "_marked").c_str())) {
      battery_.set_marked_capacity_ah(storage_.getFloat((key_prefix + "_marked").c_str()));
      loaded = true;
    }
    
    if (storage_.isKey((key_prefix + "_current").c_str())) {
      battery_.set_current_capacity_ah(storage_.getFloat((key_prefix + "_current").c_str()));
      loaded = true;
    }
    
    if (storage_.isKey((key_prefix + "_charge").c_str())) {
      battery_.set_charge_efficiency(storage_.getFloat((key_prefix + "_charge").c_str()));
      loaded = true;
    }
    
    if (storage_.isKey((key_prefix + "_discharge").c_str())) {
      battery_.set_discharge_efficiency(storage_.getFloat((key_prefix + "_discharge").c_str()));
      loaded = true;
    }
    
    if (storage_.isKey((key_prefix + "_ah").c_str())) {
      battery_.set_ah(storage_.getFloat((key_prefix + "_ah").c_str()));
      last_persisted_ah_ = battery_.ah();
      loaded = true;
    }
    
    storage_.end();
    return loaded;
  }

  /**
   * @brief Get the battery being monitored
   */
  Battery& battery() { return battery_; }
  const Battery& battery() const { return battery_; }

  /**
   * @brief Get the sensor being used
   */
  ISensor& sensor() { return sensor_; }
  const ISensor& sensor() const { return sensor_; }

 private:
  Battery& battery_;
  ISensor& sensor_;
  IStorageProvider& storage_;
  
  unsigned long last_integration_ms_;
  unsigned long last_persist_ms_;
  double last_persisted_ah_;
};

}  // namespace sensesp
