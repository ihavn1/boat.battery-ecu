#include <unity.h>
#include <Arduino.h>
#include "sensors/i_temperature_sensor.h"

using namespace sensesp;

// Mock temperature sensor for testing
class MockTemperatureSensor : public ITemperatureSensor {
 public:
  MockTemperatureSensor() 
      : initialized_(false), 
        device_count_(0),
        temperature_value_(25.0f) {}

  bool begin() override {
    initialized_ = true;
    return true;
  }

  void requestTemperatures() override {
    // Mock implementation - no-op
  }

  float getTemperature(uint8_t index = 0) override {
    if (index >= device_count_) {
      return -127.0f;  // Error value
    }
    return temperature_value_;
  }

  uint8_t getDeviceCount() override {
    return device_count_;
  }

  // Test helpers
  void setDeviceCount(uint8_t count) { device_count_ = count; }
  void setTemperature(float temp) { temperature_value_ = temp; }
  bool isInitialized() const { return initialized_; }

 private:
  bool initialized_;
  uint8_t device_count_;
  float temperature_value_;
};

void setUp(void) {}
void tearDown(void) {}

void test_temperature_sensor_interface_begin() {
    MockTemperatureSensor sensor;
    TEST_ASSERT_FALSE(sensor.isInitialized());
    TEST_ASSERT_TRUE(sensor.begin());
    TEST_ASSERT_TRUE(sensor.isInitialized());
}

void test_temperature_sensor_interface_default_device_count() {
    MockTemperatureSensor sensor;
    TEST_ASSERT_EQUAL_UINT8(0, sensor.getDeviceCount());
}

void test_temperature_sensor_interface_set_device_count() {
    MockTemperatureSensor sensor;
    sensor.setDeviceCount(2);
    TEST_ASSERT_EQUAL_UINT8(2, sensor.getDeviceCount());
}

void test_temperature_sensor_interface_default_temperature() {
    MockTemperatureSensor sensor;
    sensor.setDeviceCount(1);
    TEST_ASSERT_EQUAL_FLOAT(25.0f, sensor.getTemperature(0));
}

void test_temperature_sensor_interface_set_temperature() {
    MockTemperatureSensor sensor;
    sensor.setDeviceCount(1);
    sensor.setTemperature(30.5f);
    TEST_ASSERT_EQUAL_FLOAT(30.5f, sensor.getTemperature(0));
}

void test_temperature_sensor_interface_invalid_index() {
    MockTemperatureSensor sensor;
    sensor.setDeviceCount(1);
    // Request sensor at index 1 when only 1 sensor (index 0) exists
    TEST_ASSERT_EQUAL_FLOAT(-127.0f, sensor.getTemperature(1));
}

void test_temperature_sensor_interface_request_temperatures() {
    MockTemperatureSensor sensor;
    sensor.setDeviceCount(1);
    sensor.setTemperature(22.0f);
    
    // Request should not change the temperature in mock
    sensor.requestTemperatures();
    TEST_ASSERT_EQUAL_FLOAT(22.0f, sensor.getTemperature(0));
}

void test_temperature_sensor_interface_multiple_devices() {
    MockTemperatureSensor sensor;
    sensor.setDeviceCount(3);
    
    // All devices return same temperature in mock
    TEST_ASSERT_EQUAL_FLOAT(25.0f, sensor.getTemperature(0));
    TEST_ASSERT_EQUAL_FLOAT(25.0f, sensor.getTemperature(1));
    TEST_ASSERT_EQUAL_FLOAT(25.0f, sensor.getTemperature(2));
}

void test_temperature_sensor_interface_typical_house_battery() {
    MockTemperatureSensor sensor;
    sensor.setDeviceCount(1);
    sensor.setTemperature(28.5f);  // Typical battery temperature
    
    sensor.requestTemperatures();
    float temp = sensor.getTemperature(0);
    
    TEST_ASSERT_EQUAL_FLOAT(28.5f, temp);
    TEST_ASSERT_GREATER_THAN(0.0f, temp);
    TEST_ASSERT_LESS_THAN(60.0f, temp);
}

void test_temperature_sensor_interface_cold_temperature() {
    MockTemperatureSensor sensor;
    sensor.setDeviceCount(1);
    sensor.setTemperature(-10.0f);  // Cold weather
    
    TEST_ASSERT_EQUAL_FLOAT(-10.0f, sensor.getTemperature(0));
}

void test_temperature_sensor_interface_hot_temperature() {
    MockTemperatureSensor sensor;
    sensor.setDeviceCount(1);
    sensor.setTemperature(55.0f);  // Hot battery
    
    TEST_ASSERT_EQUAL_FLOAT(55.0f, sensor.getTemperature(0));
}

void test_temperature_sensor_interface_error_value() {
    MockTemperatureSensor sensor;
    // No devices set (count = 0)
    
    // Requesting any index should return error
    TEST_ASSERT_EQUAL_FLOAT(-127.0f, sensor.getTemperature(0));
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_temperature_sensor_interface_begin);
    RUN_TEST(test_temperature_sensor_interface_default_device_count);
    RUN_TEST(test_temperature_sensor_interface_set_device_count);
    RUN_TEST(test_temperature_sensor_interface_default_temperature);
    RUN_TEST(test_temperature_sensor_interface_set_temperature);
    RUN_TEST(test_temperature_sensor_interface_invalid_index);
    RUN_TEST(test_temperature_sensor_interface_request_temperatures);
    RUN_TEST(test_temperature_sensor_interface_multiple_devices);
    RUN_TEST(test_temperature_sensor_interface_typical_house_battery);
    RUN_TEST(test_temperature_sensor_interface_cold_temperature);
    RUN_TEST(test_temperature_sensor_interface_hot_temperature);
    RUN_TEST(test_temperature_sensor_interface_error_value);
    
    UNITY_END();
}

void loop() {}
