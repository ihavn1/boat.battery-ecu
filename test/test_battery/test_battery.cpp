#include <unity.h>
#include <Arduino.h>
#include "battery.h"
#include "battery_factory.h"

using namespace sensesp;

// Test battery configuration
BatteryConfig createTestConfig() {
    return BatteryConfig(
        "Test Battery",
        "test",
        200.0f,  // marked capacity
        200.0f,  // initial Ah
        "test.voltage",
        "test.current",
        "test.power",
        "test.ah",
        "test.soc"
    );
}

void setUp(void) {}
void tearDown(void) {}

void test_battery_initialization() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    TEST_ASSERT_EQUAL_STRING("Test Battery", battery.name());
    TEST_ASSERT_EQUAL_STRING("test", battery.chip_name());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 200.0f, battery.ah());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 200.0f, battery.marked_capacity_ah());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 200.0f, battery.current_capacity_ah());
}

void test_battery_voltage_current_power() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.update_readings(12.5f, 10.0f, 125.0f);
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 12.5f, battery.voltage());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, battery.current());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 125.0f, battery.power());
}

void test_battery_temperature() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.set_temperature(25.5f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 25.5f, battery.temperature());
}

void test_battery_soc_at_full() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.set_ah(200.0);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, battery.soc());
}

void test_battery_soc_at_half() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.set_ah(100.0);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, battery.soc());
}

void test_battery_soc_at_empty() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.set_ah(0.0);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, battery.soc());
}

void test_battery_is_charging() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.update_readings(13.5f, 10.0f, 135.0f);
    TEST_ASSERT_TRUE(battery.is_charging());
    TEST_ASSERT_FALSE(battery.is_discharging());
}

void test_battery_is_discharging() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.update_readings(12.0f, -5.0f, -60.0f);
    TEST_ASSERT_TRUE(battery.is_discharging());
    TEST_ASSERT_FALSE(battery.is_charging());
}

void test_battery_is_idle() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.update_readings(12.5f, 0.0f, 0.0f);
    TEST_ASSERT_FALSE(battery.is_charging());
    TEST_ASSERT_FALSE(battery.is_discharging());
}

void test_battery_integrate_charging() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.set_ah(100.0);
    battery.integrate_current(10.0f, 3600000);  // 10A for 1 hour = 10Ah
    
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 110.0f, battery.ah());
}

void test_battery_integrate_discharging() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.set_ah(100.0);
    battery.integrate_current(-10.0f, 3600000);  // -10A for 1 hour = -10Ah
    
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 90.0f, battery.ah());
}

void test_battery_capacity_management() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.set_marked_capacity_ah(200.0f);
    battery.set_current_capacity_ah(180.0f);
    
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 200.0f, battery.marked_capacity_ah());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 180.0f, battery.current_capacity_ah());
}

void test_battery_efficiency_settings() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.set_charge_efficiency(95.0f);
    battery.set_discharge_efficiency(98.0f);
    
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 95.0f, battery.charge_efficiency());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 98.0f, battery.discharge_efficiency());
}

void test_battery_is_fully_charged() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.set_ah(200.0);
    TEST_ASSERT_TRUE(battery.is_fully_charged());
    
    battery.set_ah(180.0);
    TEST_ASSERT_FALSE(battery.is_fully_charged());
}

void test_battery_is_empty() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.set_ah(0.0);
    TEST_ASSERT_TRUE(battery.is_empty());
    
    battery.set_ah(10.0);
    TEST_ASSERT_FALSE(battery.is_empty());
}

void test_battery_is_critically_low() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.set_ah(40.0);  // 20% of 200Ah
    TEST_ASSERT_TRUE(battery.is_critically_low());
    
    battery.set_ah(50.0);  // 25%
    TEST_ASSERT_FALSE(battery.is_critically_low());
}

void test_battery_health_percentage() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.set_marked_capacity_ah(200.0f);
    battery.set_current_capacity_ah(180.0f);
    
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 90.0f, battery.health_percentage());
}

void test_battery_health_percentage_full() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.set_marked_capacity_ah(200.0f);
    battery.set_current_capacity_ah(200.0f);
    
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, battery.health_percentage());
}

void test_battery_has_ah_changed_significantly() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    battery.set_ah(100.0);
    
    // Small change (< 0.5 Ah)
    TEST_ASSERT_FALSE(battery.has_ah_changed_significantly(100.3));
    
    // Large change (>= 0.5 Ah)
    TEST_ASSERT_TRUE(battery.has_ah_changed_significantly(99.0));
}

void test_battery_typical_house_scenario() {
    BatteryConfig houseConfig(
        "House Battery",
        "house",
        200.0f,
        200.0f,
        "electrical.batteries.house.voltage",
        "electrical.batteries.house.current",
        "electrical.batteries.house.capacity.nominal",
        "electrical.batteries.house.capacity.remaining",
        "electrical.batteries.house.capacity.stateOfCharge"
    );
    
    Battery house(houseConfig);
    
    // Fully charged
    house.update_readings(13.2f, 0.0f, 0.0f);
    house.set_ah(200.0);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, house.soc());
    TEST_ASSERT_TRUE(house.is_fully_charged());
    
    // Simulate 10 hours of 5A discharge
    for (int i = 0; i < 10; i++) {
        house.integrate_current(-5.0f, 3600000);  // 1 hour
    }
    
    // Should have 150Ah remaining (75% SOC)
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 150.0f, house.ah());
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 75.0f, house.soc());
    TEST_ASSERT_FALSE(house.is_critically_low());
}

void test_battery_typical_starter_scenario() {
    BatteryConfig starterConfig(
        "Starter Battery",
        "start",
        110.0f,
        110.0f,
        "electrical.batteries.starter.voltage",
        "electrical.batteries.starter.current",
        "electrical.batteries.starter.capacity.nominal",
        "electrical.batteries.starter.capacity.remaining",
        "electrical.batteries.starter.capacity.stateOfCharge"
    );
    
    Battery starter(starterConfig);
    
    // Fully charged
    starter.set_ah(110.0);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, starter.soc());
    
    // Simulate engine start (200A for 3 seconds)
    starter.update_readings(11.5f, -200.0f, -2300.0f);
    starter.integrate_current(-200.0f, 3000);  // 3 seconds
    
    // Should have consumed ~0.167Ah (200A * 3s / 3600s)
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 109.8f, starter.ah());
    TEST_ASSERT_TRUE(starter.is_discharging());
}

void test_battery_degraded_capacity() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    // Simulate aged battery
    battery.set_marked_capacity_ah(200.0f);
    battery.set_current_capacity_ah(170.0f);  // 85% health
    
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 85.0f, battery.health_percentage());
    
    // Set to half of current capacity
    battery.set_ah(85.0);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, battery.soc());
}

void test_battery_config_accessor() {
    BatteryConfig config = createTestConfig();
    Battery battery(config);
    
    const BatteryConfig& retrieved = battery.config();
    TEST_ASSERT_EQUAL_STRING("Test Battery", retrieved.name());
    TEST_ASSERT_EQUAL_STRING("test", retrieved.chip_name());
}

void test_factory_battery_retains_configuration() {
    Battery* battery = BatteryFactory::createHouseBattery();

    TEST_ASSERT_EQUAL_STRING("House Battery", battery->name());
    TEST_ASSERT_EQUAL_STRING("electrical.batteries.house.capacity.remaining",
                             battery->config().remaining_capacity_path());
    TEST_ASSERT_EQUAL_FLOAT(200.0f, battery->marked_capacity_ah());

    delete battery;
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_battery_initialization);
    RUN_TEST(test_battery_voltage_current_power);
    RUN_TEST(test_battery_temperature);
    RUN_TEST(test_battery_soc_at_full);
    RUN_TEST(test_battery_soc_at_half);
    RUN_TEST(test_battery_soc_at_empty);
    RUN_TEST(test_battery_is_charging);
    RUN_TEST(test_battery_is_discharging);
    RUN_TEST(test_battery_is_idle);
    RUN_TEST(test_battery_integrate_charging);
    RUN_TEST(test_battery_integrate_discharging);
    RUN_TEST(test_battery_capacity_management);
    RUN_TEST(test_battery_efficiency_settings);
    RUN_TEST(test_battery_is_fully_charged);
    RUN_TEST(test_battery_is_empty);
    RUN_TEST(test_battery_is_critically_low);
    RUN_TEST(test_battery_health_percentage);
    RUN_TEST(test_battery_health_percentage_full);
    RUN_TEST(test_battery_has_ah_changed_significantly);
    RUN_TEST(test_battery_typical_house_scenario);
    RUN_TEST(test_battery_typical_starter_scenario);
    RUN_TEST(test_battery_degraded_capacity);
    RUN_TEST(test_battery_config_accessor);
    RUN_TEST(test_factory_battery_retains_configuration);
    
    UNITY_END();
}

void loop() {}
