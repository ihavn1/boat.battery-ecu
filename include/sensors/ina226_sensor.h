#pragma once

#include "i_sensor.h"
#include "INA226.h"

namespace sensesp {

/**
 * @brief INA226 implementation of ISensor interface
 * 
 * Wraps the INA226 library to implement the sensor interface.
 * Configured with shunt resistance, current LSB, and averaging.
 */
class INA226Sensor : public ISensor {
 public:
  /**
   * @param ina Reference to INA226 hardware object
   * @param shunt_resistance Shunt resistor value in ohms (e.g., 0.0075)
   * @param current_lsb_ma Current LSB in mA (e.g., 0.250)
   * @param average_samples Number of samples to average (e.g., INA226_256_SAMPLES)
   */
  INA226Sensor(INA226& ina, float shunt_resistance, float current_lsb_ma, 
               int average_samples = INA226_256_SAMPLES)
      : ina_(ina), 
        shunt_resistance_(shunt_resistance),
        current_lsb_ma_(current_lsb_ma),
        average_samples_(average_samples) {}

  bool begin() override {
    if (!ina_.begin()) {
      return false;
    }
    ina_.configure(shunt_resistance_, current_lsb_ma_);
    ina_.setAverage(average_samples_);
    return true;
  }

  float getBusVoltage() override {
    return ina_.getBusVoltage();
  }

  float getCurrent() override {
    return ina_.getCurrent();
  }

  float getPower() override {
    return ina_.getPower();
  }

 private:
  INA226& ina_;
  float shunt_resistance_;
  float current_lsb_ma_;
  int average_samples_;
};

}  // namespace sensesp
