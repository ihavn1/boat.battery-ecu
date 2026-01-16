// Helper declarations for battery sensor setup
#pragma once

#include "sensors/i_sensor.h"
#include "battery_config.h"
#include "battery.h"
#include "battery_monitor.h"
#include "storage/i_storage_provider.h"

namespace sensesp {

/**
 * @brief Setup battery monitoring using new SOLID architecture
 * 
 * Creates Battery domain object, BatteryMonitor orchestrator,
 * and Signal K outputs/inputs following SOLID principles.
 * 
 * @param sensor Sensor implementation (INA226, INA219, etc.)
 * @param read_interval Sensor read interval in milliseconds
 * @param config Battery configuration (paths, capacity, etc.)
 * @param storage Storage backend for persisting state
 * @return BatteryMonitor* Pointer to the battery monitor for lifecycle management
 */
BatteryMonitor* setupBatterySensor(ISensor& sensor, unsigned int read_interval,
                                   const BatteryConfig& config, IStorageProvider& storage);

}  // namespace sensesp