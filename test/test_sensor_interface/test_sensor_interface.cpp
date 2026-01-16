#include <unity.h>
#include <Arduino.h>
#include "sensors/i_sensor.h"

using namespace sensesp;

// ============================================================================
// Mock Sensor for Testing
// ============================================================================

class MockSensor : public ISensor {
 public:
  MockSensor(float voltage, float current, float power, bool begin_result = true)
      : voltage_(voltage), current_(current), power_(power), 
        begin_result_(begin_result), begin_called_(false) {}

  bool begin() override {
    begin_called_ = true;
    return begin_result_;
  }

  float getBusVoltage() override {
    voltage_call_count_++;
    return voltage_;
  }

  float getCurrent() override {
    current_call_count_++;
    return current_;
  }

  float getPower() override {
    power_call_count_++;
    return power_;
  }

  // Test helpers
  void setVoltage(float v) { voltage_ = v; }
  void setCurrent(float c) { current_ = c; }
  void setPower(float p) { power_ = p; }
  bool wasBeginCalled() const { return begin_called_; }
  int getVoltageCallCount() const { return voltage_call_count_; }
  int getCurrentCallCount() const { return current_call_count_; }
  int getPowerCallCount() const { return power_call_count_; }

 private:
  float voltage_;
  float current_;
  float power_;
  bool begin_result_;
  bool begin_called_;
  int voltage_call_count_ = 0;
  int current_call_count_ = 0;
  int power_call_count_ = 0;
};

// ============================================================================
// ISensor Interface Tests
// ============================================================================

void test_sensor_interface_begin() {
  MockSensor sensor(12.5f, 5.0f, 62.5f, true);
  
  TEST_ASSERT_FALSE(sensor.wasBeginCalled());
  TEST_ASSERT_TRUE(sensor.begin());
  TEST_ASSERT_TRUE(sensor.wasBeginCalled());
}

void test_sensor_interface_begin_failure() {
  MockSensor sensor(12.5f, 5.0f, 62.5f, false);
  
  TEST_ASSERT_FALSE(sensor.begin());
  TEST_ASSERT_TRUE(sensor.wasBeginCalled());
}

void test_sensor_interface_voltage() {
  MockSensor sensor(13.2f, 5.0f, 66.0f);
  sensor.begin();
  
  float voltage = sensor.getBusVoltage();
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 13.2f, voltage);
  TEST_ASSERT_EQUAL_INT(1, sensor.getVoltageCallCount());
}

void test_sensor_interface_current() {
  MockSensor sensor(12.5f, -10.5f, -131.25f);
  sensor.begin();
  
  float current = sensor.getCurrent();
  TEST_ASSERT_FLOAT_WITHIN(0.01f, -10.5f, current);
  TEST_ASSERT_EQUAL_INT(1, sensor.getCurrentCallCount());
}

void test_sensor_interface_power() {
  MockSensor sensor(12.5f, 5.0f, 62.5f);
  sensor.begin();
  
  float power = sensor.getPower();
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 62.5f, power);
  TEST_ASSERT_EQUAL_INT(1, sensor.getPowerCallCount());
}

void test_sensor_interface_multiple_reads() {
  MockSensor sensor(12.5f, 5.0f, 62.5f);
  sensor.begin();
  
  // Read multiple times
  for (int i = 0; i < 5; i++) {
    sensor.getBusVoltage();
    sensor.getCurrent();
    sensor.getPower();
  }
  
  TEST_ASSERT_EQUAL_INT(5, sensor.getVoltageCallCount());
  TEST_ASSERT_EQUAL_INT(5, sensor.getCurrentCallCount());
  TEST_ASSERT_EQUAL_INT(5, sensor.getPowerCallCount());
}

void test_sensor_interface_value_changes() {
  MockSensor sensor(12.5f, 5.0f, 62.5f);
  sensor.begin();
  
  // Initial read
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 12.5f, sensor.getBusVoltage());
  
  // Change value
  sensor.setVoltage(14.2f);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 14.2f, sensor.getBusVoltage());
  
  // Change again
  sensor.setVoltage(11.8f);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 11.8f, sensor.getBusVoltage());
}

void test_sensor_interface_charging_scenario() {
  // Simulate charging: voltage rises, current positive, power positive
  MockSensor sensor(14.4f, 20.0f, 288.0f);
  sensor.begin();
  
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 14.4f, sensor.getBusVoltage());
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 20.0f, sensor.getCurrent());
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 288.0f, sensor.getPower());
}

void test_sensor_interface_discharging_scenario() {
  // Simulate discharging: voltage drops, current negative, power negative
  MockSensor sensor(12.2f, -15.5f, -189.1f);
  sensor.begin();
  
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 12.2f, sensor.getBusVoltage());
  TEST_ASSERT_FLOAT_WITHIN(0.01f, -15.5f, sensor.getCurrent());
  TEST_ASSERT_FLOAT_WITHIN(0.01f, -189.1f, sensor.getPower());
}

void test_sensor_interface_zero_current() {
  // Battery idle: no current flow
  MockSensor sensor(12.8f, 0.0f, 0.0f);
  sensor.begin();
  
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, sensor.getCurrent());
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, sensor.getPower());
}

// ============================================================================
// Test Runner
// ============================================================================

void setup() {
    delay(2000);  // Wait for serial
    UNITY_BEGIN();
    
    // Interface contract tests
    RUN_TEST(test_sensor_interface_begin);
    RUN_TEST(test_sensor_interface_begin_failure);
    RUN_TEST(test_sensor_interface_voltage);
    RUN_TEST(test_sensor_interface_current);
    RUN_TEST(test_sensor_interface_power);
    RUN_TEST(test_sensor_interface_multiple_reads);
    RUN_TEST(test_sensor_interface_value_changes);
    
    // Scenario tests
    RUN_TEST(test_sensor_interface_charging_scenario);
    RUN_TEST(test_sensor_interface_discharging_scenario);
    RUN_TEST(test_sensor_interface_zero_current);
    
    UNITY_END();
}

void loop() {
    // Empty - tests run once in setup()
}
