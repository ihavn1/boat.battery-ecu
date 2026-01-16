#pragma once

namespace sensesp {

/**
 * @brief Interface for battery current/voltage/power sensors
 * 
 * Abstracts hardware sensor implementation to allow different sensor types
 * (INA226, INA219, hall effect sensors, etc.) to be used interchangeably.
 * 
 * This follows the Dependency Inversion Principle - high-level battery
 * monitoring code depends on this interface, not concrete implementations.
 */
class ISensor {
 public:
  virtual ~ISensor() = default;

  /**
   * @brief Initialize the sensor hardware
   * @return true if initialization successful, false otherwise
   */
  virtual bool begin() = 0;

  /**
   * @brief Read bus voltage in volts
   * @return Voltage in V
   */
  virtual float getBusVoltage() = 0;

  /**
   * @brief Read current in amperes
   * @return Current in A (positive = charging, negative = discharging)
   */
  virtual float getCurrent() = 0;

  /**
   * @brief Read power in watts
   * @return Power in W
   */
  virtual float getPower() = 0;
};

}  // namespace sensesp
