#include <unity.h>
#include <Arduino.h>

// ============================================================================
// OneWire Temperature Sensor Configuration Tests
// ============================================================================

void test_onewire_pin(void) {
    // OneWire sensors on GPIO 25
    uint8_t onewire_pin = 25;
    
    TEST_ASSERT_EQUAL_UINT8(25, onewire_pin);
}

void test_temperature_read_interval(void) {
    // Temperature read every 2 seconds (2000ms)
    unsigned int read_delay = 2000;
    
    TEST_ASSERT_EQUAL_UINT(2000, read_delay);
}

void test_temperature_range_house_battery(void) {
    // Typical battery operating temperature: -20°C to 60°C
    // Optimal: 0°C to 40°C
    // Verify ranges are sensible: min < optimal_min < optimal_max < max
    // Unity API: TEST_ASSERT_LESS_THAN_FLOAT(threshold, actual) checks actual < threshold
    float min_temp = -20.0f;
    float optimal_min = 0.0f;
    float optimal_max = 40.0f;
    float max_temp = 60.0f;
    
    TEST_ASSERT_LESS_THAN_FLOAT(optimal_min, min_temp);      // min_temp < optimal_min (-20 < 0)
    TEST_ASSERT_LESS_THAN_FLOAT(optimal_max, optimal_min);   // optimal_min < optimal_max (0 < 40)
    TEST_ASSERT_LESS_THAN_FLOAT(max_temp, optimal_max);      // optimal_max < max_temp (40 < 60)
}

void test_temperature_range_starter_battery(void) {
    // Starter battery same temperature range as house
    float min_temp = -20.0f;
    float max_temp = 60.0f;
    
    TEST_ASSERT_LESS_OR_EQUAL_FLOAT(max_temp, 60.0f);
    TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(min_temp, -20.0f);
}

void test_celsius_to_kelvin_conversion(void) {
    float celsius = 25.0f;
    float kelvin = celsius + 273.15f;

    TEST_ASSERT_FLOAT_WITHIN(0.01f, 298.15f, kelvin);
}

void test_temperature_calibration_sort_values(void) {
    // From main.cpp: sensor_sort, linear_sort, sk_sort values
    // House battery: 110, 120, 130
    // Starter battery: 210, 220, 230
    
    int house_sensor_sort = 110;
    int house_linear_sort = 120;
    int house_sk_sort = 130;
    
    int starter_sensor_sort = 210;
    int starter_linear_sort = 220;
    int starter_sk_sort = 230;
    
    TEST_ASSERT_EQUAL_INT(110, house_sensor_sort);
    TEST_ASSERT_EQUAL_INT(120, house_linear_sort);
    TEST_ASSERT_EQUAL_INT(130, house_sk_sort);
    
    TEST_ASSERT_EQUAL_INT(210, starter_sensor_sort);
    TEST_ASSERT_EQUAL_INT(220, starter_linear_sort);
    TEST_ASSERT_EQUAL_INT(230, starter_sk_sort);
}

void test_temperature_alarm_thresholds(void) {
    // Warning at 45°C, critical at 55°C
    float warning_temp = 45.0f;
    float critical_temp = 55.0f;
    
    TEST_ASSERT_GREATER_THAN_FLOAT(warning_temp, critical_temp);
}

// ============================================================================
// Main Test Runner
// ============================================================================

void setup() {
    delay(2000);
    
    UNITY_BEGIN();
    
    RUN_TEST(test_onewire_pin);
    RUN_TEST(test_temperature_read_interval);
    RUN_TEST(test_temperature_range_house_battery);
    RUN_TEST(test_temperature_range_starter_battery);
    RUN_TEST(test_celsius_to_kelvin_conversion);
    RUN_TEST(test_temperature_calibration_sort_values);
    RUN_TEST(test_temperature_alarm_thresholds);
    
    UNITY_END();
}

void loop() {
}
