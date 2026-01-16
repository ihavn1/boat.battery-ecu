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
 public:
  /**
   * @brief Create an INA226 sensor
   * @param address I2C address (e.g., 0x40, 0x41)
   * @param shunt_resistance Shunt resistance in ohms (default 0.0075)
   * @param current_lsb Current LSB in mA (default 0.250)
   * @param averaging Sample averaging mode (default 256 samples)
   * @return INA226 sensor instance
   */
  static ISensor* createINA226(uint8_t address, 
                                float shunt_resistance = 0.0075f,
                                float current_lsb = 0.250f,
                                int averaging = INA226_256_SAMPLES) {
    static INA226 ina_device(address);
    return new INA226Sensor(ina_device, shunt_resistance, current_lsb, averaging);
  }

  /**
   * @brief Create house battery sensor (INA226 at 0x40)
   * @return House battery sensor
   */
  static ISensor* createHouseBatterySensor() {
    return createINA226(0x40, 0.0075f, 0.250f, INA226_256_SAMPLES);
  }

  /**
   * @brief Create starter battery sensor (INA226 at 0x41)
   * @return Starter battery sensor
   */
  static ISensor* createStarterBatterySensor() {
    return createINA226(0x41, 0.0075f, 0.250f, INA226_256_SAMPLES);
  }

  /**
   * @brief Create sensor from type and parameters
   * @param type Sensor type
   * @param address I2C address
   * @param shunt_resistance Shunt resistance in ohms
   * @param current_lsb Current LSB in mA
   * @return Sensor instance or nullptr if type not supported
   */
  static ISensor* create(SensorType type, uint8_t address,
                         float shunt_resistance = 0.0075f,
                         float current_lsb = 0.250f) {
    switch (type) {
      case SensorType::INA226:
        return createINA226(address, shunt_resistance, current_lsb);
      
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
