#pragma once

#include "i_temperature_sensor.h"
#include "sensesp_onewire/onewire_temperature.h"

namespace sensesp {

/**
 * @brief Dallas OneWire temperature sensor implementation
 * 
 * Wraps DallasTemperatureSensors to implement ITemperatureSensor interface.
 * This allows the system to swap Dallas sensors for other temperature sensors
 * without changing dependent code.
 */
class DallasTemperatureSensor : public ITemperatureSensor {
 public:
  /**
   * @brief Construct a Dallas OneWire temperature sensor wrapper
   * @param dts Pointer to DallasTemperatureSensors instance
   */
  explicit DallasTemperatureSensor(sensesp::onewire::DallasTemperatureSensors* dts)
      : dts_(dts) {}

  bool begin() override {
    // DallasTemperatureSensors doesn't have a begin() method,
    // initialization is done in its constructor
    return dts_ != nullptr;
  }

  void requestTemperatures() override {
    if (dts_) {
      dts_->request_temperatures();
    }
  }

  float getTemperature(uint8_t index = 0) override {
    if (dts_) {
      return dts_->get_temperature(index);
    }
    return -127.0f;  // Error value for Dallas sensors
  }

  uint8_t getDeviceCount() override {
    if (dts_) {
      return dts_->get_device_count();
    }
    return 0;
  }

 private:
  sensesp::onewire::DallasTemperatureSensors* dts_;
};

}  // namespace sensesp
