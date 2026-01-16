// Helper declarations for battery sensor setup
#pragma once

#include "sensors/i_sensor.h"
#include "battery_config.h"
#include "storage/i_storage_provider.h"

namespace sensesp {

/**
 * @brief Setup battery monitoring with any ISensor implementation
 * 
 * Creates the sensor reading pipeline, amp-hour integration, SOC calculation,
 * and Signal K outputs/inputs for a single battery.
 * 
 * @param sensor Sensor implementation (INA226, INA219, etc.)
 * @param read_interval Sensor read interval in milliseconds
 * @param config Battery configuration (paths, capacity, etc.)
 * @param storage Storage backend for persisting state
 */
void setupBatterySensor(ISensor& sensor, unsigned int read_interval,
                        const BatteryConfig& config, IStorageProvider& storage);

}  // namespace sensesp