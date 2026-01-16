#pragma once

#include <Arduino.h>
#include "sensors/i_temperature_sensor.h"

namespace sensesp {

/**
 * @brief Configuration for temperature calibration
 */
struct TemperatureCalibration {
  float slope;      // Linear calibration slope (default 1.0)
  float offset;     // Linear calibration offset in °C (default 0.0)
  
  TemperatureCalibration() : slope(1.0f), offset(0.0f) {}
  TemperatureCalibration(float s, float o) : slope(s), offset(o) {}
  
  float apply(float raw_temp) const {
    return raw_temp * slope + offset;
  }
};

/**
 * @brief Orchestrates temperature monitoring
 * 
 * Coordinates temperature sensor reading and calibration.
 * Follows Single Responsibility Principle by focusing only
 * on temperature monitoring orchestration.
 */
class TemperatureMonitor {
 public:
  /**
   * @brief Construct temperature monitor
   * @param sensor Temperature sensor interface
   * @param device_index Sensor device index (for multi-device buses)
   * @param name Human-readable name
   */
  TemperatureMonitor(ITemperatureSensor& sensor, uint8_t device_index = 0, 
                     const char* name = "Temperature")
      : sensor_(sensor),
        device_index_(device_index),
        name_(name),
        temperature_(0.0f),
        calibration_() {}

  /**
   * @brief Initialize sensor
   * @return true if successful
   */
  bool begin() {
    return sensor_.begin();
  }

  /**
   * @brief Request temperature measurement
   * For sensors that need time to measure (like Dallas OneWire)
   */
  void request_measurement() {
    sensor_.requestTemperatures();
  }

  /**
   * @brief Read temperature from sensor and apply calibration
   * @return Calibrated temperature in °C, or -127.0 on error
   */
  float read_temperature() {
    float raw = sensor_.getTemperature(device_index_);
    if (raw == -127.0f) {
      return raw;  // Error
    }
    
    temperature_ = calibration_.apply(raw);
    return temperature_;
  }

  /**
   * @brief Get last read temperature
   */
  float temperature() const { return temperature_; }

  /**
   * @brief Get raw (uncalibrated) temperature
   */
  float raw_temperature() const {
    return sensor_.getTemperature(device_index_);
  }

  /**
   * @brief Set calibration parameters
   */
  void set_calibration(float slope, float offset) {
    calibration_ = TemperatureCalibration(slope, offset);
  }

  /**
   * @brief Set calibration from config
   */
  void set_calibration(const TemperatureCalibration& cal) {
    calibration_ = cal;
  }

  /**
   * @brief Get calibration parameters
   */
  const TemperatureCalibration& calibration() const { 
    return calibration_; 
  }

  /**
   * @brief Get sensor name
   */
  const char* name() const { return name_; }

  /**
   * @brief Get device index
   */
  uint8_t device_index() const { return device_index_; }

  /**
   * @brief Get number of devices on the bus
   */
  uint8_t device_count() const {
    return sensor_.getDeviceCount();
  }

  /**
   * @brief Check if temperature is within normal range
   * @param min_temp Minimum acceptable temperature (default -20°C)
   * @param max_temp Maximum acceptable temperature (default 60°C)
   */
  bool is_temperature_normal(float min_temp = -20.0f, float max_temp = 60.0f) const {
    return temperature_ >= min_temp && temperature_ <= max_temp;
  }

  /**
   * @brief Check if temperature is critically high
   * @param critical_temp Critical temperature threshold (default 55°C)
   */
  bool is_critically_hot(float critical_temp = 55.0f) const {
    return temperature_ >= critical_temp;
  }

  /**
   * @brief Check if temperature is critically low
   * @param critical_temp Critical temperature threshold (default -10°C)
   */
  bool is_critically_cold(float critical_temp = -10.0f) const {
    return temperature_ <= critical_temp;
  }

 private:
  ITemperatureSensor& sensor_;
  uint8_t device_index_;
  const char* name_;
  float temperature_;
  TemperatureCalibration calibration_;
};

}  // namespace sensesp
