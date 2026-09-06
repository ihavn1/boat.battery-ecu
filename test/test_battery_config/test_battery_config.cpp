#include <unity.h>
#include "battery_config.h"

using namespace sensesp;

void setUp(void) {}
void tearDown(void) {}

void test_battery_config_house_name() {
    BatteryConfig config("House Battery", "house", 200.0f, 200.0f,
                         "electrical.batteries.house.voltage",
                         "electrical.batteries.house.current",
                         "electrical.batteries.house.capacity.nominal",
                         "electrical.batteries.house.capacity.remaining",
                         "electrical.batteries.house.capacity.stateOfCharge");
    TEST_ASSERT_EQUAL_STRING("House Battery", config.name());
}

void test_battery_config_chip_name() {
    BatteryConfig config("House Battery", "house", 200.0f, 200.0f,
                         "electrical.batteries.house.voltage",
                         "electrical.batteries.house.current",
                         "electrical.batteries.house.capacity.nominal",
                         "electrical.batteries.house.capacity.remaining",
                         "electrical.batteries.house.capacity.stateOfCharge");
    TEST_ASSERT_EQUAL_STRING("house", config.chip_name());
}

void test_battery_config_marked_capacity() {
    BatteryConfig config("House Battery", "house", 200.0f, 180.0f,
                         "electrical.batteries.house.voltage",
                         "electrical.batteries.house.current",
                         "electrical.batteries.house.capacity.nominal",
                         "electrical.batteries.house.capacity.remaining",
                         "electrical.batteries.house.capacity.stateOfCharge");
    TEST_ASSERT_EQUAL_FLOAT(200.0f, config.marked_capacity_ah());
}

void test_battery_config_initial_ah() {
    BatteryConfig config("House Battery", "house", 200.0f, 180.0f,
                         "electrical.batteries.house.voltage",
                         "electrical.batteries.house.current",
                         "electrical.batteries.house.capacity.nominal",
                         "electrical.batteries.house.capacity.remaining",
                         "electrical.batteries.house.capacity.stateOfCharge");
    TEST_ASSERT_EQUAL_FLOAT(180.0f, config.initial_ah());
}

void test_battery_config_voltage_path() {
    BatteryConfig config("House Battery", "house", 200.0f, 200.0f,
                         "electrical.batteries.house.voltage",
                         "electrical.batteries.house.current",
                         "electrical.batteries.house.capacity.nominal",
                         "electrical.batteries.house.capacity.remaining",
                         "electrical.batteries.house.capacity.stateOfCharge");
    TEST_ASSERT_EQUAL_STRING("electrical.batteries.house.voltage", config.voltage_path());
}

void test_battery_config_current_path() {
    BatteryConfig config("House Battery", "house", 200.0f, 200.0f,
                         "electrical.batteries.house.voltage",
                         "electrical.batteries.house.current",
                         "electrical.batteries.house.capacity.nominal",
                         "electrical.batteries.house.capacity.remaining",
                         "electrical.batteries.house.capacity.stateOfCharge");
    TEST_ASSERT_EQUAL_STRING("electrical.batteries.house.current", config.current_path());
}

void test_battery_config_nominal_capacity_path() {
    BatteryConfig config("House Battery", "house", 200.0f, 200.0f,
                         "electrical.batteries.house.voltage",
                         "electrical.batteries.house.current",
                         "electrical.batteries.house.capacity.nominal",
                         "electrical.batteries.house.capacity.remaining",
                         "electrical.batteries.house.capacity.stateOfCharge");
    TEST_ASSERT_EQUAL_STRING("electrical.batteries.house.capacity.nominal",
                             config.nominal_capacity_path());
}

void test_battery_config_remaining_capacity_path() {
    BatteryConfig config("House Battery", "house", 200.0f, 200.0f,
                         "electrical.batteries.house.voltage",
                         "electrical.batteries.house.current",
                         "electrical.batteries.house.capacity.nominal",
                         "electrical.batteries.house.capacity.remaining",
                         "electrical.batteries.house.capacity.stateOfCharge");
    TEST_ASSERT_EQUAL_STRING("electrical.batteries.house.capacity.remaining",
                             config.remaining_capacity_path());
}

void test_battery_config_soc_path() {
    BatteryConfig config("House Battery", "house", 200.0f, 200.0f,
                         "electrical.batteries.house.voltage",
                         "electrical.batteries.house.current",
                         "electrical.batteries.house.capacity.nominal",
                         "electrical.batteries.house.capacity.remaining",
                         "electrical.batteries.house.capacity.stateOfCharge");
    TEST_ASSERT_EQUAL_STRING("electrical.batteries.house.capacity.stateOfCharge", config.soc_path());
}

void test_battery_config_starter_battery() {
    BatteryConfig config("Starter Battery", "start", 110.0f, 110.0f,
                         "electrical.batteries.starter.voltage",
                         "electrical.batteries.starter.current",
                         "electrical.batteries.starter.power",
                         "electrical.batteries.starter.ah",
                         "electrical.batteries.starter.stateOfCharge");
    TEST_ASSERT_EQUAL_STRING("Starter Battery", config.name());
    TEST_ASSERT_EQUAL_STRING("start", config.chip_name());
    TEST_ASSERT_EQUAL_FLOAT(110.0f, config.marked_capacity_ah());
    TEST_ASSERT_EQUAL_FLOAT(110.0f, config.initial_ah());
}

void test_battery_config_different_initial_and_marked_capacity() {
    BatteryConfig config("House Battery", "house", 200.0f, 150.0f,
                         "electrical.batteries.house.voltage",
                         "electrical.batteries.house.current",
                         "electrical.batteries.house.power",
                         "electrical.batteries.house.ah",
                         "electrical.batteries.house.stateOfCharge");
    TEST_ASSERT_EQUAL_FLOAT(200.0f, config.marked_capacity_ah());
    TEST_ASSERT_EQUAL_FLOAT(150.0f, config.initial_ah());
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_battery_config_house_name);
    RUN_TEST(test_battery_config_chip_name);
    RUN_TEST(test_battery_config_marked_capacity);
    RUN_TEST(test_battery_config_initial_ah);
    RUN_TEST(test_battery_config_voltage_path);
    RUN_TEST(test_battery_config_current_path);
    RUN_TEST(test_battery_config_nominal_capacity_path);
    RUN_TEST(test_battery_config_remaining_capacity_path);
    RUN_TEST(test_battery_config_soc_path);
    RUN_TEST(test_battery_config_starter_battery);
    RUN_TEST(test_battery_config_different_initial_and_marked_capacity);
    
    UNITY_END();
}

void loop() {}
