#pragma once

#include <Arduino.h>

namespace sensesp {

/**
 * @brief Configuration data for a battery
 * 
 * Encapsulates all static configuration for a battery, separating
 * configuration from runtime state. This follows the Single Responsibility
 * Principle - this class only holds configuration data.
 */
class BatteryConfig {
 public:
  /**
   * @param name Display name (e.g., "House Battery", "Starter Battery")
   * @param chip_name Short NVS key prefix (e.g., "house", "start")
   * @param marked_capacity_ah Nameplate capacity in Ah
   * @param initial_ah Initial amp-hours at startup (typically full capacity)
   * @param voltage_path Signal K path for voltage
   * @param current_path Signal K path for current
    * @param nominal_capacity_path Signal K path for nominal capacity (J)
    * @param remaining_capacity_path Signal K path for remaining capacity (J)
   * @param soc_path Signal K path for state of charge
    * @param actual_capacity_path Signal K path for actual capacity (J)
    * @param nominal_voltage Battery nominal voltage used for Ah/J conversion
   */
  BatteryConfig(const char* name,
                const char* chip_name,
                float marked_capacity_ah,
                float initial_ah,
                const char* voltage_path,
                const char* current_path,
                const char* nominal_capacity_path,
                const char* remaining_capacity_path,
                const char* soc_path,
                const char* actual_capacity_path = nullptr,
                float nominal_voltage = 12.0f)
      : name_(name),
        chip_name_(chip_name),
        marked_capacity_ah_(marked_capacity_ah),
        initial_ah_(initial_ah),
        voltage_path_(voltage_path),
        current_path_(current_path),
        nominal_capacity_path_(nominal_capacity_path),
        remaining_capacity_path_(remaining_capacity_path),
        soc_path_(soc_path),
        actual_capacity_path_(actual_capacity_path),
        nominal_voltage_(nominal_voltage) {}

  // Getters
  const char* name() const { return name_; }
  const char* chip_name() const { return chip_name_; }
  float marked_capacity_ah() const { return marked_capacity_ah_; }
  float initial_ah() const { return initial_ah_; }
  const char* voltage_path() const { return voltage_path_; }
  const char* current_path() const { return current_path_; }
  const char* nominal_capacity_path() const { return nominal_capacity_path_; }
  const char* remaining_capacity_path() const { return remaining_capacity_path_; }
  const char* soc_path() const { return soc_path_; }
  const char* actual_capacity_path() const { return actual_capacity_path_; }
  float nominal_voltage() const { return nominal_voltage_; }

 private:
  const char* name_;
  const char* chip_name_;
  float marked_capacity_ah_;
  float initial_ah_;
  const char* voltage_path_;
  const char* current_path_;
  const char* nominal_capacity_path_;
  const char* remaining_capacity_path_;
  const char* soc_path_;
  const char* actual_capacity_path_;
  float nominal_voltage_;
};

}  // namespace sensesp
