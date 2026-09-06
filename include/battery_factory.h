#pragma once

#include <Arduino.h>
#include "battery.h"
#include "battery_config.h"

namespace sensesp {

/**
 * @brief Factory for creating Battery instances
 * 
 * Encapsulates Battery creation logic, following the Open/Closed Principle
 * by allowing extension through configuration without modifying existing code.
 */
class BatteryFactory {
 public:
  /**
   * @brief Create a battery from configuration
   * @param config Battery configuration
   * @return Newly created Battery instance
   */
  static Battery* create(const BatteryConfig& config) {
    return new Battery(config);
  }

  /**
   * @brief Create house battery with standard configuration
   * @param capacity_ah Battery capacity in Ah
   * @return House battery instance
   */
  static Battery* createHouseBattery(float capacity_ah = 200.0f) {
    BatteryConfig config(
        "House Battery",
        "house",
        capacity_ah,
        capacity_ah,
        "electrical.batteries.house.voltage",
        "electrical.batteries.house.current",
        "electrical.batteries.house.capacity.nominal",
        "electrical.batteries.house.capacity.remaining",
        "electrical.batteries.house.capacity.stateOfCharge",
        "electrical.batteries.house.capacity.actual"
    );
    return create(config);
  }

  /**
   * @brief Create starter battery with standard configuration
   * @param capacity_ah Battery capacity in Ah
   * @return Starter battery instance
   */
  static Battery* createStarterBattery(float capacity_ah = 110.0f) {
    BatteryConfig config(
        "Starter Battery",
        "start",
        capacity_ah,
        capacity_ah,
        "electrical.batteries.starter.voltage",
        "electrical.batteries.starter.current",
        "electrical.batteries.starter.capacity.nominal",
        "electrical.batteries.starter.capacity.remaining",
        "electrical.batteries.starter.capacity.stateOfCharge",
        "electrical.batteries.starter.capacity.actual"
    );
    return create(config);
  }

  /**
   * @brief Create a custom battery
   * @param name Display name
   * @param chip_name Short NVS key prefix
   * @param capacity_ah Battery capacity
   * @param base_path Signal K base path (e.g., "electrical.batteries.aux")
   * @return Custom battery instance
   */
  static Battery* createCustomBattery(const char* name, const char* chip_name,
                                       float capacity_ah, const char* base_path) {
    String base(base_path);
    BatteryConfig config(
        name,
        chip_name,
        capacity_ah,
        capacity_ah,
        (base + ".voltage").c_str(),
        (base + ".current").c_str(),
        (base + ".capacity.nominal").c_str(),
        (base + ".capacity.remaining").c_str(),
        (base + ".capacity.stateOfCharge").c_str(),
        (base + ".capacity.actual").c_str()
    );
    return create(config);
  }
};

}  // namespace sensesp
