#pragma once

namespace sensesp {

/**
 * @brief Interface for temperature sensors
 * 
 * This interface allows the system to work with different temperature sensor
 * implementations (Dallas OneWire, DHT22, BME280, etc.) without tight coupling
 * to specific hardware. Follows the Dependency Inversion Principle.
 */
class ITemperatureSensor {
 public:
  virtual ~ITemperatureSensor() = default;

  /**
   * @brief Initialize the temperature sensor
   * @return true if initialization successful, false otherwise
   */
  virtual bool begin() = 0;

  /**
   * @brief Request temperature measurement from sensor
   * 
   * For sensors that require time to measure (like Dallas OneWire),
   * this initiates the measurement. Call getTemperature() after appropriate delay.
   */
  virtual void requestTemperatures() = 0;

  /**
   * @brief Get temperature reading in Celsius
   * @param index Sensor index (for sensors with multiple devices on one bus)
   * @return Temperature in degrees Celsius, or -127.0 if reading failed
   */
  virtual float getTemperature(uint8_t index = 0) = 0;

  /**
   * @brief Get number of temperature devices detected
   * @return Number of devices on the bus/sensor
   */
  virtual uint8_t getDeviceCount() = 0;
};

}  // namespace sensesp
