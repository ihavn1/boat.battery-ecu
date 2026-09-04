#include <unity.h>
#include <Arduino.h>
#include <map>
#include "shutdown_coordinator.h"
#include "battery_monitor.h"
#include "battery.h"
#include "sensors/i_sensor.h"
#include "storage/i_storage_provider.h"

using namespace sensesp;

namespace {

class FakeClock : public IClock {
 public:
  uint64_t now_ms() const override { return now_ms_; }
  uint64_t now_ms_ = 0;
};

class FakeSleepController : public ISleepController {
 public:
  void enter_deep_sleep() override { sleep_count_++; }
  unsigned int sleep_count_ = 0;
};

class FakeSensor : public ISensor {
 public:
  bool begin() override { return true; }
  float getBusVoltage() override { return 12.0f; }
  float getCurrent() override { return 0.0f; }
  float getPower() override { return 0.0f; }
};

class FakeStorage : public IStorageProvider {
 public:
  bool begin(const char* ns, bool ro = false) override { return true; }
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
};

BatteryConfig createTestConfig(const char* chip_name) {
  return BatteryConfig("Test", chip_name, 200.0f, 200.0f, "test.v", "test.a",
                       "test.w", "test.ah", "test.soc");
}

}  // namespace

void setUp(void) {}
void tearDown(void) {}

void test_shutdown_coordinator_ignores_high_signal() {
  BatteryConfig house_config = createTestConfig("house");
  BatteryConfig starter_config = createTestConfig("start");
  Battery house_battery(house_config);
  Battery starter_battery(starter_config);
  FakeSensor sensor;
  FakeStorage storage;
  BatteryMonitor house_monitor(house_battery, sensor, storage);
  BatteryMonitor starter_monitor(starter_battery, sensor, storage);
  FakeClock clock;
  FakeSleepController sleeper;

  ShutdownCoordinator coordinator(clock, house_monitor, starter_monitor, sleeper, 0);
  coordinator.sample(false);

  TEST_ASSERT_FALSE(coordinator.is_complete());
  TEST_ASSERT_EQUAL_UINT(0, sleeper.sleep_count_);
}

void test_shutdown_coordinator_saves_and_sleeps_with_zero_debounce() {
  BatteryConfig house_config = createTestConfig("house");
  BatteryConfig starter_config = createTestConfig("start");
  Battery house_battery(house_config);
  Battery starter_battery(starter_config);
  FakeSensor sensor;
  FakeStorage storage;
  BatteryMonitor house_monitor(house_battery, sensor, storage);
  BatteryMonitor starter_monitor(starter_battery, sensor, storage);
  FakeClock clock;
  FakeSleepController sleeper;

  ShutdownCoordinator coordinator(clock, house_monitor, starter_monitor, sleeper, 0);
  coordinator.sample(true);

  TEST_ASSERT_TRUE(coordinator.is_complete());
  TEST_ASSERT_EQUAL_UINT(1, sleeper.sleep_count_);
}

void test_shutdown_coordinator_waits_for_debounce() {
  BatteryConfig house_config = createTestConfig("house");
  BatteryConfig starter_config = createTestConfig("start");
  Battery house_battery(house_config);
  Battery starter_battery(starter_config);
  FakeSensor sensor;
  FakeStorage storage;
  BatteryMonitor house_monitor(house_battery, sensor, storage);
  BatteryMonitor starter_monitor(starter_battery, sensor, storage);
  FakeClock clock;
  FakeSleepController sleeper;

  ShutdownCoordinator coordinator(clock, house_monitor, starter_monitor, sleeper, 100);

  clock.now_ms_ = 0;
  coordinator.sample(true);
  TEST_ASSERT_FALSE(coordinator.is_complete());

  clock.now_ms_ = 50;
  coordinator.sample(true);
  TEST_ASSERT_FALSE(coordinator.is_complete());

  clock.now_ms_ = 100;
  coordinator.sample(true);
  TEST_ASSERT_TRUE(coordinator.is_complete());
  TEST_ASSERT_EQUAL_UINT(1, sleeper.sleep_count_);
}

void test_shutdown_coordinator_cancels_on_high_signal_before_debounce() {
  BatteryConfig house_config = createTestConfig("house");
  BatteryConfig starter_config = createTestConfig("start");
  Battery house_battery(house_config);
  Battery starter_battery(starter_config);
  FakeSensor sensor;
  FakeStorage storage;
  BatteryMonitor house_monitor(house_battery, sensor, storage);
  BatteryMonitor starter_monitor(starter_battery, sensor, storage);
  FakeClock clock;
  FakeSleepController sleeper;

  ShutdownCoordinator coordinator(clock, house_monitor, starter_monitor, sleeper, 100);

  clock.now_ms_ = 0;
  coordinator.sample(true);
  clock.now_ms_ = 50;
  coordinator.sample(false);  // Pin went high again: debounce should reset

  clock.now_ms_ = 100;
  coordinator.sample(true);  // Restarts debounce, not enough time elapsed
  TEST_ASSERT_FALSE(coordinator.is_complete());

  clock.now_ms_ = 200;
  coordinator.sample(true);
  TEST_ASSERT_TRUE(coordinator.is_complete());
}

void test_shutdown_coordinator_persists_both_batteries() {
  BatteryConfig house_config = createTestConfig("house");
  BatteryConfig starter_config = createTestConfig("start");
  Battery house_battery(house_config);
  Battery starter_battery(starter_config);
  house_battery.set_ah(123.0);
  starter_battery.set_ah(45.0);
  FakeSensor sensor;
  FakeStorage storage;
  BatteryMonitor house_monitor(house_battery, sensor, storage);
  BatteryMonitor starter_monitor(starter_battery, sensor, storage);
  FakeClock clock;
  FakeSleepController sleeper;

  ShutdownCoordinator coordinator(clock, house_monitor, starter_monitor, sleeper, 0);
  coordinator.sample(true);

  BatteryConfig house_config2 = createTestConfig("house");
  BatteryConfig starter_config2 = createTestConfig("start");
  Battery house_battery2(house_config2);
  Battery starter_battery2(starter_config2);
  BatteryMonitor house_monitor2(house_battery2, sensor, storage);
  BatteryMonitor starter_monitor2(starter_battery2, sensor, storage);

  TEST_ASSERT_FLOAT_WITHIN(0.01, 123.0, house_monitor2.battery().ah());
  TEST_ASSERT_FLOAT_WITHIN(0.01, 45.0, starter_monitor2.battery().ah());
}

void test_shutdown_coordinator_is_idempotent() {
  BatteryConfig house_config = createTestConfig("house");
  BatteryConfig starter_config = createTestConfig("start");
  Battery house_battery(house_config);
  Battery starter_battery(starter_config);
  FakeSensor sensor;
  FakeStorage storage;
  BatteryMonitor house_monitor(house_battery, sensor, storage);
  BatteryMonitor starter_monitor(starter_battery, sensor, storage);
  FakeClock clock;
  FakeSleepController sleeper;

  ShutdownCoordinator coordinator(clock, house_monitor, starter_monitor, sleeper, 0);
  coordinator.sample(true);
  coordinator.sample(true);
  coordinator.sample(true);

  TEST_ASSERT_EQUAL_UINT(1, sleeper.sleep_count_);
}

void setup() {
  delay(2000);
  UNITY_BEGIN();

  RUN_TEST(test_shutdown_coordinator_ignores_high_signal);
  RUN_TEST(test_shutdown_coordinator_saves_and_sleeps_with_zero_debounce);
  RUN_TEST(test_shutdown_coordinator_waits_for_debounce);
  RUN_TEST(test_shutdown_coordinator_cancels_on_high_signal_before_debounce);
  RUN_TEST(test_shutdown_coordinator_persists_both_batteries);
  RUN_TEST(test_shutdown_coordinator_is_idempotent);

  UNITY_END();
}

void loop() {}
