#include <unity.h>
#include <Arduino.h>
#include "battery_monitor.h"
#include "battery.h"
#include "sensors/i_sensor.h"
#include "storage/i_storage_provider.h"
#include <map>

using namespace sensesp;

// Mock sensor
class MockSensor : public ISensor {
 public:
  MockSensor() : voltage_(12.0f), current_(0.0f), power_(0.0f), initialized_(false) {}
  
  bool begin() override { initialized_ = true; return true; }
  float getBusVoltage() override { return voltage_; }
  float getCurrent() override { return current_; }
  float getPower() override { return power_; }
  
  void setReadings(float v, float a, float w) {
    voltage_ = v;
    current_ = a;
    power_ = w;
  }
  
  bool isInitialized() const { return initialized_; }
  
 private:
  float voltage_, current_, power_;
  bool initialized_;
};

// Mock storage
class MockStorage : public IStorageProvider {
 public:
  bool begin(const char* ns, bool ro = false) override {
    current_ns_ = ns;
    return true;
  }
  void end() override {}
  bool isKey(const char* key) override {
    return data_.find(String(key)) != data_.end();
  }
  float getFloat(const char* key, float def = 0.0f) override {
    auto it = data_.find(String(key));
    return (it != data_.end()) ? it->second : def;
  }
  size_t putFloat(const char* key, float val) override {
    data_[String(key)] = val;
    return sizeof(float);
  }
  String getString(const char* key, const String& def = "") override { return def; }
  size_t putString(const char* key, const String& val) override { return 0; }
  bool remove(const char* key) override { return false; }
  bool clear() override { data_.clear(); return true; }
  
 private:
  std::map<String, float> data_;
  String current_ns_;
};

BatteryConfig createTestConfig() {
    return BatteryConfig("Test", "test", 200.0f, 200.0f,
                         "test.v", "test.a", "test.w", "test.ah", "test.soc");
}

void setUp(void) {}
void tearDown(void) {}

void test_battery_monitor_initialization() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    MockSensor sensor;
    MockStorage storage;
    
    BatteryMonitor monitor(battery, sensor, storage);
    
    TEST_ASSERT_TRUE(monitor.begin());
    TEST_ASSERT_TRUE(sensor.isInitialized());
}

void test_battery_monitor_update_readings() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    MockSensor sensor;
    MockStorage storage;
    
    BatteryMonitor monitor(battery, sensor, storage);
    
    sensor.setReadings(13.2f, 10.5f, 138.6f);
    monitor.update_readings();
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 13.2f, battery.voltage());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.5f, battery.current());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 138.6f, battery.power());
}

void test_battery_monitor_integrate() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    MockSensor sensor;
    MockStorage storage;
    
    BatteryMonitor monitor(battery, sensor, storage);
    
    battery.set_ah(100.0);
    sensor.setReadings(12.0f, 10.0f, 120.0f);
    monitor.update_readings();
    
    // Directly integrate current for 10 seconds (10000 ms)
    // 10A * 10s = 100 As = 0.0278 Ah
    battery.integrate_current(10.0f, 10000);
    
    TEST_ASSERT_FLOAT_WITHIN(0.001, 100.0278, battery.ah());
}

void test_battery_monitor_save_load_state() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    MockSensor sensor;
    MockStorage storage;
    
    BatteryMonitor monitor(battery, sensor, storage);
    
    battery.set_ah(150.0);
    battery.set_charge_efficiency(95.0f);
    battery.set_discharge_efficiency(98.0f);
    battery.set_current_capacity_ah(190.0f);
    
    TEST_ASSERT_TRUE(monitor.save_state());
    
    // Create new battery/monitor and load state
    Battery battery2(config);
    BatteryMonitor monitor2(battery2, sensor, storage);
    
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 150.0f, battery2.ah());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 95.0f, battery2.charge_efficiency());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 98.0f, battery2.discharge_efficiency());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 190.0f, battery2.current_capacity_ah());
}

void test_battery_monitor_maybe_persist_no_change() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    MockSensor sensor;
    MockStorage storage;
    
    BatteryMonitor monitor(battery, sensor, storage);
    
    battery.set_ah(100.0);
    monitor.save_state();
    
    storage.clear();
    
    // Small change - should not persist (below threshold)
    battery.set_ah(100.2);
    monitor.maybe_persist(false, 10000000, 0.5);  // 10M ms = enough time elapsed
    
    TEST_ASSERT_FALSE(storage.isKey("test_ah"));
}

void test_battery_monitor_maybe_persist_significant_change() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    MockSensor sensor;
    MockStorage storage;
    
    BatteryMonitor monitor(battery, sensor, storage);
    
    battery.set_ah(100.0);
    monitor.save_state();
    
    storage.clear();
    
    // Large change - should persist
    battery.set_ah(101.0);
    monitor.maybe_persist(false, 0, 0.5);
    
    TEST_ASSERT_TRUE(storage.isKey("test_ah"));
}

void test_battery_monitor_maybe_persist_force() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    MockSensor sensor;
    MockStorage storage;
    
    BatteryMonitor monitor(battery, sensor, storage);
    
    battery.set_ah(100.0);
    monitor.save_state();
    
    storage.clear();
    
    // Force persist even without change
    monitor.maybe_persist(true);
    
    TEST_ASSERT_TRUE(storage.isKey("test_ah"));
}

void test_battery_monitor_accessors() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    MockSensor sensor;
    MockStorage storage;
    
    BatteryMonitor monitor(battery, sensor, storage);
    
    TEST_ASSERT_EQUAL_PTR(&battery, &monitor.battery());
    TEST_ASSERT_EQUAL_PTR(&sensor, &monitor.sensor());
}

void test_battery_monitor_typical_scenario() {
    BatteryConfig config("House", "house", 200.0f, 200.0f,
                         "house.v", "house.a", "house.w", "house.ah", "house.soc");
    Battery battery(config);
    MockSensor sensor;
    MockStorage storage;
    
    BatteryMonitor monitor(battery, sensor, storage);
    monitor.begin();
    
    // Simulate charging
    sensor.setReadings(13.8f, 15.0f, 207.0f);
    monitor.update_readings();
    
    TEST_ASSERT_TRUE(battery.is_charging());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 13.8f, battery.voltage());
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_battery_monitor_initialization);
    RUN_TEST(test_battery_monitor_update_readings);
    RUN_TEST(test_battery_monitor_integrate);
    RUN_TEST(test_battery_monitor_save_load_state);
    RUN_TEST(test_battery_monitor_maybe_persist_no_change);
    RUN_TEST(test_battery_monitor_maybe_persist_significant_change);
    RUN_TEST(test_battery_monitor_maybe_persist_force);
    RUN_TEST(test_battery_monitor_accessors);
    RUN_TEST(test_battery_monitor_typical_scenario);
    
    UNITY_END();
}

void loop() {}
