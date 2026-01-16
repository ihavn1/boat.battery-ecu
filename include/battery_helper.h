// Helper declarations for battery sensor setup
#pragma once

#include "sensors/i_sensor.h"
#include "battery_config.h"

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
 */
void setupBatterySensor(ISensor& sensor, unsigned int read_interval,
                        const BatteryConfig& config);

}  // namespace sensesp