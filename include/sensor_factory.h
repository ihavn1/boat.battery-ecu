#pragma once

#include "sensors/i_sensor.h"
#include "sensors/ina226_sensor.h"
#include "INA226.h"

namespace sensesp {

/**
 * @brief Sensor types supported by the factory
 */
enum class SensorType {
  INA226,
  INA219,
  // Future: INA3221, ACS712, etc.
};

/**
 * @brief Factory for creating sensor instances
 * 
 * Encapsulates sensor creation logic, following the Open/Closed Principle
 * by allowing new sensor types to be added without modifying existing code.
 */
class SensorFactory {
 private:
  // Store INA226 hardware instances (one per address)
  static INA226 house_battery_ina_;
  static INA226 starter_battery_ina_;

 public:
  /**
   * @brief Create an INA226 sensor with specified hardware instance
   * @param ina Reference to INA226 hardware object
   * @param shunt_resistance Shunt resistance in ohms (default 0.0075)
   * @param current_lsb Current LSB in mA (default 0.250)
   * @param averaging Sample averaging mode (default 256 samples)
   * @return INA226 sensor instance
   */
  static ISensor* createINA226(INA226& ina,
                                float shunt_resistance = 0.0075f,
                                float current_lsb = 0.250f,
                                int averaging = INA226_256_SAMPLES) {
    return new INA226Sensor(ina, shunt_resistance, current_lsb, averaging);
  }

  /**
   * @brief Create house battery sensor (INA226 at 0x40)
   * @return House battery sensor
   */
  static ISensor* createHouseBatterySensor() {
    return createINA226(house_battery_ina_, 0.0075f, 0.250f, INA226_256_SAMPLES);
  }

  /**
   * @brief Create starter battery sensor (INA226 at 0x41)
   * @return Starter battery sensor
   */
  static ISensor* createStarterBatterySensor() {
    return createINA226(starter_battery_ina_, 0.0075f, 0.250f, INA226_256_SAMPLES);
  }

  /**
   * @brief Create sensor from type and parameters
   * @param type Sensor type
   * @param ina Reference to INA226 hardware object
   * @param shunt_resistance Shunt resistance in ohms
   * @param current_lsb Current LSB in mA
   * @return Sensor instance or nullptr if type not supported
   */
  static ISensor* create(SensorType type, INA226& ina,
                         float shunt_resistance = 0.0075f,
                         float current_lsb = 0.250f) {
    switch (type) {
      case SensorType::INA226:
        return createINA226(ina, shunt_resistance, current_lsb);
      
      case SensorType::INA219:
        // Future implementation
        return nullptr;
      
      default:
        return nullptr;
    }
  }

  /**
   * @brief Auto-detect sensor type at I2C address
   * @param address I2C address to probe
   * @return Detected sensor type or nullptr if not detected
   * 
   * Note: This is a placeholder for future auto-detection logic
   */
  static SensorType detectSensorType(uint8_t address) {
    // Future: Probe I2C device ID registers
    return SensorType::INA226;  // Default assumption
  }
};

}  // namespace sensesp
