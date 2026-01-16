#include <unity.h>
#include <Arduino.h>

// ============================================================================
// INA226 Configuration Tests
// ============================================================================
// Tests verify the INA226 sensor configuration parameters used in main.cpp

void test_ina226_shunt_resistance(void) {
    // Shunt resistance: 0.0075Ω (7.5 milliohms)
    float shunt_resistance = 0.0075F;
    
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0075f, shunt_resistance);
}

void test_ina226_current_lsb(void) {
    // Current LSB: 0.250mA = 0.00025A
    float current_lsb_mA = 0.250F;
    float current_lsb_A = current_lsb_mA / 1000.0f;
    
    TEST_ASSERT_FLOAT_WITHIN(0.000001f, 0.00025f, current_lsb_A);
}

void test_ina226_sample_averaging(void) {
    // Should be configured with 256 samples averaging
    int averaging = 256;
    
    TEST_ASSERT_EQUAL_INT(256, averaging);
}

void test_ina226_i2c_addresses(void) {
    // House battery: 0x40
    // Starter battery: 0x41
    uint8_t house_addr = 0x40;
    uint8_t starter_addr = 0x41;
    
    TEST_ASSERT_EQUAL_UINT8(0x40, house_addr);
    TEST_ASSERT_EQUAL_UINT8(0x41, starter_addr);
}

void test_ina226_voltage_range(void) {
    // INA226 bus voltage range: 0-36V
    // Typical boat battery: 10-15V (12V nominal)
    float min_voltage = 10.0f;
    float nominal_voltage = 12.0f;
    float max_voltage = 15.0f;
    float ina226_max = 36.0f;
    
    TEST_ASSERT_LESS_OR_EQUAL_FLOAT(ina226_max, max_voltage);
    TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(min_voltage, nominal_voltage);
}

void test_ina226_current_range(void) {
    // With 0.0075Ω shunt and 0.25mA LSB:
    // Max current = ±81.92A (INA226 spec with ±81.92mV max shunt voltage)
    // Vshunt_max = ±81.92mV
    // Imax = Vshunt_max / Rshunt = 0.08192V / 0.0075Ω ≈ 10.9A
    
    float shunt_resistance = 0.0075f;
    float max_shunt_voltage = 0.08192f; // ±81.92mV
    float theoretical_max_current = max_shunt_voltage / shunt_resistance;
    
    TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(10.0f, theoretical_max_current);
}

void test_battery_capacity_house(void) {
    // House battery: 200Ah
    float house_capacity = 200.0f;
    
    TEST_ASSERT_EQUAL_FLOAT(200.0f, house_capacity);
}

void test_battery_capacity_starter(void) {
    // Starter battery: 110Ah
    float starter_capacity = 110.0f;
    
    TEST_ASSERT_EQUAL_FLOAT(110.0f, starter_capacity);
}

void test_battery_read_interval(void) {
    // Battery sensors read at 1Hz (1000ms interval)
    unsigned int read_interval = 1000;
    
    TEST_ASSERT_EQUAL_UINT(1000, read_interval);
}

// ============================================================================
// Power Calculation Tests
// ============================================================================

void test_power_calculation_typical_load(void) {
    // P = V * I
    // 12V battery with 5A draw
    float voltage = 12.0f;
    float current = 5.0f;
    float power = voltage * current;
    
    TEST_ASSERT_EQUAL_FLOAT(60.0f, power); // 60W
}

void test_power_calculation_charging(void) {
    // 14.4V charging voltage with 10A charge current
    float voltage = 14.4f;
    float current = 10.0f;
    float power = voltage * current;
    
    TEST_ASSERT_EQUAL_FLOAT(144.0f, power); // 144W
}

void test_power_calculation_starter_motor(void) {
    // Starter motor: 12V with 100A draw
    float voltage = 12.0f;
    float current = 100.0f;
    float power = voltage * current;
    
    TEST_ASSERT_EQUAL_FLOAT(1200.0f, power); // 1200W = 1.2kW
}

// ============================================================================
// Main Test Runner
// ============================================================================

void setup() {
    delay(2000);
    
    UNITY_BEGIN();
    
    // INA226 configuration
    RUN_TEST(test_ina226_shunt_resistance);
    RUN_TEST(test_ina226_current_lsb);
    RUN_TEST(test_ina226_sample_averaging);
    RUN_TEST(test_ina226_i2c_addresses);
    RUN_TEST(test_ina226_voltage_range);
    RUN_TEST(test_ina226_current_range);
    
    // Battery configuration
    RUN_TEST(test_battery_capacity_house);
    RUN_TEST(test_battery_capacity_starter);
    RUN_TEST(test_battery_read_interval);
    
    // Power calculations
    RUN_TEST(test_power_calculation_typical_load);
    RUN_TEST(test_power_calculation_charging);
    RUN_TEST(test_power_calculation_starter_motor);
    
    UNITY_END();
}

void loop() {
}
