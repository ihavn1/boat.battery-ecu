#include <unity.h>
#include <Arduino.h>
#include "temperature_monitor.h"
#include "sensors/i_temperature_sensor.h"

using namespace sensesp;

// Mock temperature sensor
class MockTempSensor : public ITemperatureSensor {
 public:
  MockTempSensor() : temperature_(25.0f), device_count_(1), initialized_(false) {}
  
  bool begin() override { initialized_ = true; return true; }
  void requestTemperatures() override {}
  float getTemperature(uint8_t index = 0) override {
    if (index >= device_count_) return -127.0f;
    return temperature_;
  }
  uint8_t getDeviceCount() override { return device_count_; }
  
  void setTemperature(float temp) { temperature_ = temp; }
  void setDeviceCount(uint8_t count) { device_count_ = count; }
  bool isInitialized() const { return initialized_; }
  
 private:
  float temperature_;
  uint8_t device_count_;
  bool initialized_;
};

void setUp(void) {}
void tearDown(void) {}

void test_temperature_monitor_initialization() {
    MockTempSensor sensor;
    TemperatureMonitor monitor(sensor, 0, "Test Temp");
    
    TEST_ASSERT_TRUE(monitor.begin());
    TEST_ASSERT_TRUE(sensor.isInitialized());
    TEST_ASSERT_EQUAL_STRING("Test Temp", monitor.name());
}

void test_temperature_monitor_read_temperature() {
    MockTempSensor sensor;
    TemperatureMonitor monitor(sensor);
    
    sensor.setTemperature(28.5f);
    float temp = monitor.read_temperature();
    
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 28.5f, temp);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 28.5f, monitor.temperature());
}

void test_temperature_monitor_calibration() {
    MockTempSensor sensor;
    TemperatureMonitor monitor(sensor);
    
    sensor.setTemperature(25.0f);
    monitor.set_calibration(1.0f, 2.5f);  // Add 2.5°C offset
    
    float temp = monitor.read_temperature();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 27.5f, temp);
}

void test_temperature_monitor_calibration_slope() {
    MockTempSensor sensor;
    TemperatureMonitor monitor(sensor);
    
    sensor.setTemperature(20.0f);
    monitor.set_calibration(0.95f, 1.0f);  // 95% slope + 1°C offset
    
    float temp = monitor.read_temperature();
    // 20 * 0.95 + 1.0 = 20.0
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 20.0f, temp);
}

void test_temperature_monitor_device_index() {
    MockTempSensor sensor;
    sensor.setDeviceCount(3);
    
    TemperatureMonitor monitor(sensor, 1, "Sensor 2");
    
    TEST_ASSERT_EQUAL(1, monitor.device_index());
    TEST_ASSERT_EQUAL(3, monitor.device_count());
}

void test_temperature_monitor_error_handling() {
    MockTempSensor sensor;
    sensor.setDeviceCount(1);
    
    TemperatureMonitor monitor(sensor, 5);  // Invalid index
    
    float temp = monitor.read_temperature();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, -127.0f, temp);
}

void test_temperature_monitor_is_temperature_normal() {
    MockTempSensor sensor;
    TemperatureMonitor monitor(sensor);
    
    sensor.setTemperature(25.0f);
    monitor.read_temperature();
    TEST_ASSERT_TRUE(monitor.is_temperature_normal());
    
    sensor.setTemperature(-25.0f);
    monitor.read_temperature();
    TEST_ASSERT_FALSE(monitor.is_temperature_normal());
    
    sensor.setTemperature(70.0f);
    monitor.read_temperature();
    TEST_ASSERT_FALSE(monitor.is_temperature_normal());
}

void test_temperature_monitor_is_critically_hot() {
    MockTempSensor sensor;
    TemperatureMonitor monitor(sensor);
    
    sensor.setTemperature(50.0f);
    monitor.read_temperature();
    TEST_ASSERT_FALSE(monitor.is_critically_hot());
    
    sensor.setTemperature(60.0f);
    monitor.read_temperature();
    TEST_ASSERT_TRUE(monitor.is_critically_hot());
}

void test_temperature_monitor_is_critically_cold() {
    MockTempSensor sensor;
    TemperatureMonitor monitor(sensor);
    
    sensor.setTemperature(5.0f);
    monitor.read_temperature();
    TEST_ASSERT_FALSE(monitor.is_critically_cold());
    
    sensor.setTemperature(-15.0f);
    monitor.read_temperature();
    TEST_ASSERT_TRUE(monitor.is_critically_cold());
}

void test_temperature_monitor_request_measurement() {
    MockTempSensor sensor;
    TemperatureMonitor monitor(sensor);
    
    // Should not throw or crash
    monitor.request_measurement();
    TEST_ASSERT_TRUE(true);
}

void test_temperature_monitor_calibration_struct() {
    TemperatureCalibration cal(0.98f, 1.5f);
    
    float adjusted = cal.apply(30.0f);
    // 30 * 0.98 + 1.5 = 30.9
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 30.9f, adjusted);
}

void test_temperature_monitor_typical_house_battery() {
    MockTempSensor sensor;
    TemperatureMonitor monitor(sensor, 0, "House Battery Temp");
    
    monitor.begin();
    sensor.setTemperature(32.5f);
    monitor.set_calibration(1.0f, -2.0f);  // Calibration correction
    
    float temp = monitor.read_temperature();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 30.5f, temp);
    TEST_ASSERT_TRUE(monitor.is_temperature_normal());
    TEST_ASSERT_FALSE(monitor.is_critically_hot());
}

void test_temperature_monitor_typical_starter_battery() {
    MockTempSensor sensor;
    TemperatureMonitor monitor(sensor, 1, "Starter Battery Temp");
    
    sensor.setDeviceCount(2);
    monitor.begin();
    
    sensor.setTemperature(28.0f);
    float temp = monitor.read_temperature();
    
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 28.0f, temp);
    TEST_ASSERT_EQUAL(1, monitor.device_index());
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_temperature_monitor_initialization);
    RUN_TEST(test_temperature_monitor_read_temperature);
    RUN_TEST(test_temperature_monitor_calibration);
    RUN_TEST(test_temperature_monitor_calibration_slope);
    RUN_TEST(test_temperature_monitor_device_index);
    RUN_TEST(test_temperature_monitor_error_handling);
    RUN_TEST(test_temperature_monitor_is_temperature_normal);
    RUN_TEST(test_temperature_monitor_is_critically_hot);
    RUN_TEST(test_temperature_monitor_is_critically_cold);
    RUN_TEST(test_temperature_monitor_request_measurement);
    RUN_TEST(test_temperature_monitor_calibration_struct);
    RUN_TEST(test_temperature_monitor_typical_house_battery);
    RUN_TEST(test_temperature_monitor_typical_starter_battery);
    
    UNITY_END();
}

void loop() {}
