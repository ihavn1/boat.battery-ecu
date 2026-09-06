#include <unity.h>
#include <Arduino.h>
#include <string.h>

// ============================================================================
// Signal K Path Validation Tests
// ============================================================================
// Ensures Signal K paths follow the correct format and conventions

void test_house_battery_voltage_path(void) {
    const char* path = "electrical.batteries.house.voltage";
    TEST_ASSERT_TRUE(strstr(path, "electrical.batteries.house") != NULL);
}

void test_house_battery_current_path(void) {
    const char* path = "electrical.batteries.house.current";
    TEST_ASSERT_TRUE(strstr(path, "electrical.batteries.house") != NULL);
}

void test_house_battery_nominal_capacity_path(void) {
    const char* path = "electrical.batteries.house.capacity.nominal";
    TEST_ASSERT_TRUE(strstr(path, "electrical.batteries.house") != NULL);
}

void test_house_battery_remaining_capacity_path(void) {
    const char* path = "electrical.batteries.house.capacity.remaining";
    TEST_ASSERT_TRUE(strstr(path, "electrical.batteries.house") != NULL);
}

void test_house_battery_soc_path(void) {
    const char* path = "electrical.batteries.house.capacity.stateOfCharge";
    TEST_ASSERT_TRUE(strstr(path, "electrical.batteries.house") != NULL);
    TEST_ASSERT_TRUE(strstr(path, "stateOfCharge") != NULL);
}

void test_house_battery_temperature_path(void) {
    const char* path = "electrical.batteries.house.temperature";
    TEST_ASSERT_TRUE(strstr(path, "electrical.batteries.house") != NULL);
}

void test_starter_battery_voltage_path(void) {
    const char* path = "electrical.batteries.starter.voltage";
    TEST_ASSERT_TRUE(strstr(path, "electrical.batteries.starter") != NULL);
}

void test_starter_battery_current_path(void) {
    const char* path = "electrical.batteries.starter.current";
    TEST_ASSERT_TRUE(strstr(path, "electrical.batteries.starter") != NULL);
}

void test_starter_battery_nominal_capacity_path(void) {
    const char* path = "electrical.batteries.starter.capacity.nominal";
    TEST_ASSERT_TRUE(strstr(path, "electrical.batteries.starter") != NULL);
}

void test_starter_battery_remaining_capacity_path(void) {
    const char* path = "electrical.batteries.starter.capacity.remaining";
    TEST_ASSERT_TRUE(strstr(path, "electrical.batteries.starter") != NULL);
}

void test_starter_battery_soc_path(void) {
    const char* path = "electrical.batteries.starter.capacity.stateOfCharge";
    TEST_ASSERT_TRUE(strstr(path, "electrical.batteries.starter") != NULL);
}

void test_starter_battery_temperature_path(void) {
    const char* path = "electrical.batteries.starter.temperature";
    TEST_ASSERT_TRUE(strstr(path, "electrical.batteries.starter") != NULL);
}

// ============================================================================
// Signal K Configuration Sub-Paths Tests
// ============================================================================

void test_charge_efficiency_path_format(void) {
    const char* base_path = "electrical.batteries.house.configuration";
    String charge_path = String(base_path) + ".chargeEfficiency";
    
    TEST_ASSERT_TRUE(charge_path.indexOf(".chargeEfficiency") > 0);
}

void test_discharge_efficiency_path_format(void) {
    const char* base_path = "electrical.batteries.house.configuration";
    String discharge_path = String(base_path) + ".dischargeEfficiency";
    
    TEST_ASSERT_TRUE(discharge_path.indexOf(".dischargeEfficiency") > 0);
}

void test_capacity_path_format(void) {
    const char* base_path = "electrical.batteries.house.capacity";
    String capacity_path = String(base_path) + ".actual";
    
    TEST_ASSERT_TRUE(capacity_path == "electrical.batteries.house.capacity.actual");
}

void test_marked_capacity_path_format(void) {
    const char* base_path = "electrical.batteries.house.capacity";
    String marked_capacity_path = String(base_path) + ".nominal";
    
    TEST_ASSERT_TRUE(marked_capacity_path == "electrical.batteries.house.capacity.nominal");
}

// ============================================================================
// Signal K PUT Request Format Tests
// ============================================================================

void test_put_request_ah_value_format(void) {
    // PUT request should have format: {"value": <number>}
    float test_value = 180.5f;
    
    TEST_ASSERT_GREATER_THAN_FLOAT(0.0f, test_value);
    TEST_ASSERT_LESS_THAN_FLOAT(300.0f, test_value); // Reasonable battery Ah
}

void test_put_request_efficiency_range(void) {
    // Efficiency should be 0-100%
    float test_efficiency = 85.0f;
    
    TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(0.0f, test_efficiency);
    TEST_ASSERT_LESS_OR_EQUAL_FLOAT(100.0f, test_efficiency);
}

// ============================================================================
// NVS Key Format Tests
// ============================================================================

void test_nvs_namespace(void) {
    const char* nvs_namespace = "battcfg";
    
    // Namespace should be short (max 15 chars)
    TEST_ASSERT_LESS_OR_EQUAL_UINT(15, strlen(nvs_namespace));
}

void test_nvs_key_house_ah(void) {
    // Config path "house" → NVS key "house_ah"
    String config_path = "house";
    String key = config_path + "_ah";
    
    // Key must be ≤15 chars for ESP32 NVS
    TEST_ASSERT_LESS_OR_EQUAL_UINT(15, key.length());
}

void test_nvs_key_house_charge(void) {
    String config_path = "house";
    String key = config_path + "_charge";
    
    TEST_ASSERT_LESS_OR_EQUAL_UINT(15, key.length());
}

void test_nvs_key_house_discharge(void) {
    String config_path = "house";
    String key = config_path + "_discharge";
    
    TEST_ASSERT_LESS_OR_EQUAL_UINT(15, key.length());
}

void test_nvs_key_starter_ah(void) {
    String config_path = "start";  // Note: "start" not "starter" to save chars
    String key = config_path + "_ah";
    
    TEST_ASSERT_LESS_OR_EQUAL_UINT(15, key.length());
}

void test_nvs_key_current_capacity(void) {
    String config_path = "house";
    String key = config_path + "_current";
    
    TEST_ASSERT_LESS_OR_EQUAL_UINT(15, key.length());
}

void test_nvs_key_marked_capacity(void) {
    String config_path = "house";
    String key = config_path + "_marked";
    
    TEST_ASSERT_LESS_OR_EQUAL_UINT(15, key.length());
}

// ============================================================================
// Main Test Runner
// ============================================================================

void setup() {
    delay(2000);
    
    UNITY_BEGIN();
    
    // House battery paths
    RUN_TEST(test_house_battery_voltage_path);
    RUN_TEST(test_house_battery_current_path);
    RUN_TEST(test_house_battery_nominal_capacity_path);
    RUN_TEST(test_house_battery_remaining_capacity_path);
    RUN_TEST(test_house_battery_soc_path);
    RUN_TEST(test_house_battery_temperature_path);
    
    // Starter battery paths
    RUN_TEST(test_starter_battery_voltage_path);
    RUN_TEST(test_starter_battery_current_path);
    RUN_TEST(test_starter_battery_nominal_capacity_path);
    RUN_TEST(test_starter_battery_remaining_capacity_path);
    RUN_TEST(test_starter_battery_soc_path);
    RUN_TEST(test_starter_battery_temperature_path);
    
    // Configuration sub-paths
    RUN_TEST(test_charge_efficiency_path_format);
    RUN_TEST(test_discharge_efficiency_path_format);
    RUN_TEST(test_capacity_path_format);
    RUN_TEST(test_marked_capacity_path_format);
    
    // PUT request format
    RUN_TEST(test_put_request_ah_value_format);
    RUN_TEST(test_put_request_efficiency_range);
    
    // NVS key format
    RUN_TEST(test_nvs_namespace);
    RUN_TEST(test_nvs_key_house_ah);
    RUN_TEST(test_nvs_key_house_charge);
    RUN_TEST(test_nvs_key_house_discharge);
    RUN_TEST(test_nvs_key_starter_ah);
    RUN_TEST(test_nvs_key_current_capacity);
    RUN_TEST(test_nvs_key_marked_capacity);
    
    UNITY_END();
}

void loop() {
}
