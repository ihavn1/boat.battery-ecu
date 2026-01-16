#include <unity.h>
#include "ah_calculator.h"

// Test fixtures
AmpHourCalculator* calc = nullptr;
const float TEST_CAPACITY = 100.0f;
const float TEST_INITIAL_AH = 50.0f;

void setUp(void) {
    calc = new AmpHourCalculator(TEST_INITIAL_AH, TEST_CAPACITY);
}

void tearDown(void) {
    if (calc) {
        delete calc;
        calc = nullptr;
    }
}

// ============================================================================
// Basic Functionality Tests
// ============================================================================

void test_calculator_initialization(void) {
    TEST_ASSERT_EQUAL_FLOAT(TEST_INITIAL_AH, calc->get_ah());
    TEST_ASSERT_EQUAL_FLOAT(100.0f, calc->get_charge_efficiency());
    TEST_ASSERT_EQUAL_FLOAT(100.0f, calc->get_discharge_efficiency());
    TEST_ASSERT_EQUAL_FLOAT(TEST_CAPACITY, calc->get_current_capacity_ah());
    TEST_ASSERT_EQUAL_FLOAT(TEST_CAPACITY, calc->get_marked_capacity_ah());
}

void test_calculator_set_ah(void) {
    calc->set_ah(75.5);
    TEST_ASSERT_EQUAL_FLOAT(75.5f, calc->get_ah());
    
    // Test clamping to capacity
    calc->set_ah(150.0);
    TEST_ASSERT_EQUAL_FLOAT(TEST_CAPACITY, calc->get_ah());
    
    // Test clamping to zero
    calc->set_ah(-10.0);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, calc->get_ah());
}

void test_calculator_set_charge_efficiency(void) {
    calc->set_charge_efficiency(85.0f);
    TEST_ASSERT_EQUAL_FLOAT(85.0f, calc->get_charge_efficiency());
    
    // Test clamping to 0-100%
    calc->set_charge_efficiency(150.0f);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, calc->get_charge_efficiency());
    
    calc->set_charge_efficiency(-10.0f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, calc->get_charge_efficiency());
}

void test_calculator_set_discharge_efficiency(void) {
    calc->set_discharge_efficiency(90.0f);
    TEST_ASSERT_EQUAL_FLOAT(90.0f, calc->get_discharge_efficiency());
    
    // Test clamping
    calc->set_discharge_efficiency(120.0f);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, calc->get_discharge_efficiency());
    
    calc->set_discharge_efficiency(-5.0f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, calc->get_discharge_efficiency());
}

void test_calculator_capacity_management(void) {
    // Test current capacity
    calc->set_current_capacity_ah(80.0f);
    TEST_ASSERT_EQUAL_FLOAT(80.0f, calc->get_current_capacity_ah());
    
    // Test marked capacity
    calc->set_marked_capacity_ah(100.0f);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, calc->get_marked_capacity_ah());
    
    // Test minimum capacity clamping (0.1 Ah)
    calc->set_current_capacity_ah(0.05f);
    TEST_ASSERT_EQUAL_FLOAT(0.1f, calc->get_current_capacity_ah());
    
    // Test maximum capacity clamping (10000 Ah)
    calc->set_current_capacity_ah(15000.0f);
    TEST_ASSERT_EQUAL_FLOAT(10000.0f, calc->get_current_capacity_ah());
}

void test_calculator_capacity_change_clamps_ah(void) {
    // Set Ah to 90
    calc->set_ah(90.0f);
    TEST_ASSERT_EQUAL_FLOAT(90.0f, calc->get_ah());
    
    // Reduce capacity to 80Ah
    calc->set_current_capacity_ah(80.0f);
    
    // Ah should be clamped to new capacity
    TEST_ASSERT_EQUAL_FLOAT(80.0f, calc->get_ah());
}

// ============================================================================
// Integration Tests
// ============================================================================

void test_calculator_integrate_charging(void) {
    calc->set_ah(50.0);
    
    // Charge at 10A for 1 hour (3600000 ms)
    double new_ah = calc->integrate_current(10.0, 3600000);
    
    // Should add 10Ah: 50 + 10 = 60
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 60.0f, new_ah);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 60.0f, calc->get_ah());
}

void test_calculator_integrate_discharging(void) {
    calc->set_ah(50.0);
    
    // Discharge at 10A for 1 hour
    double new_ah = calc->integrate_current(-10.0, 3600000);
    
    // Should remove 10Ah: 50 - 10 = 40
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 40.0f, new_ah);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 40.0f, calc->get_ah());
}

void test_calculator_integrate_with_charge_efficiency(void) {
    calc->set_ah(50.0);
    calc->set_charge_efficiency(85.0f);  // 85% efficient
    
    // Charge at 10A for 1 hour
    // Effective: 10A * 0.85 = 8.5A
    double new_ah = calc->integrate_current(10.0, 3600000);
    
    // Should add 8.5Ah: 50 + 8.5 = 58.5
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 58.5f, new_ah);
}

void test_calculator_integrate_with_discharge_efficiency(void) {
    calc->set_ah(50.0);
    calc->set_discharge_efficiency(95.0f);  // 95% efficient
    
    // Discharge at 10A for 1 hour
    // Effective: 10A * 0.95 = 9.5A
    double new_ah = calc->integrate_current(-10.0, 3600000);
    
    // Should remove 9.5Ah: 50 - 9.5 = 40.5
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 40.5f, new_ah);
}

void test_calculator_integrate_short_duration(void) {
    calc->set_ah(50.0);
    
    // Charge at 10A for 1 second (1000 ms)
    // 10A * (1000/3600000) hours = 10A * 0.000277778 hours = 0.00277778 Ah
    double new_ah = calc->integrate_current(10.0, 1000);
    
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 50.00277778f, new_ah);
}

void test_calculator_integrate_clamps_at_full(void) {
    calc->set_ah(99.5);
    
    // Charge at 10A for 1 hour (would go to 109.5)
    double new_ah = calc->integrate_current(10.0, 3600000);
    
    // Should clamp to capacity (100)
    TEST_ASSERT_EQUAL_FLOAT(100.0f, new_ah);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, calc->get_ah());
}

void test_calculator_integrate_clamps_at_empty(void) {
    calc->set_ah(0.5);
    
    // Discharge at 10A for 1 hour (would go to -9.5)
    double new_ah = calc->integrate_current(-10.0, 3600000);
    
    // Should clamp to 0
    TEST_ASSERT_EQUAL_FLOAT(0.0f, new_ah);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, calc->get_ah());
}

// ============================================================================
// SOC Calculation Tests
// ============================================================================

void test_calculator_soc_at_full(void) {
    calc->set_ah(100.0f);
    float soc = calc->calculate_soc();
    TEST_ASSERT_EQUAL_FLOAT(100.0f, soc);
}

void test_calculator_soc_at_half(void) {
    calc->set_ah(50.0f);
    float soc = calc->calculate_soc();
    TEST_ASSERT_EQUAL_FLOAT(50.0f, soc);
}

void test_calculator_soc_at_empty(void) {
    calc->set_ah(0.0f);
    float soc = calc->calculate_soc();
    TEST_ASSERT_EQUAL_FLOAT(0.0f, soc);
}

void test_calculator_soc_with_degraded_battery(void) {
    // Battery degraded to 80Ah
    calc->set_current_capacity_ah(80.0f);
    calc->set_ah(80.0f);  // Fully charged
    
    float soc = calc->calculate_soc();
    TEST_ASSERT_EQUAL_FLOAT(100.0f, soc);
}

void test_calculator_soc_zero_capacity(void) {
    delete calc;
    calc = new AmpHourCalculator(50.0f, 0.0f);
    
    float soc = calc->calculate_soc();
    TEST_ASSERT_EQUAL_FLOAT(0.0f, soc);
}

// ============================================================================
// Change Detection Tests
// ============================================================================

void test_calculator_has_changed_significantly(void) {
    calc->set_ah(50.0);
    
    // Change by 0.3Ah (below 0.5 threshold)
    calc->set_ah(50.3);
    TEST_ASSERT_FALSE(calc->has_changed_significantly(50.0, 0.5));
    
    // Change by 0.6Ah (above 0.5 threshold)
    calc->set_ah(50.6);
    TEST_ASSERT_TRUE(calc->has_changed_significantly(50.0, 0.5));
    
    // Negative change
    calc->set_ah(49.3);
    TEST_ASSERT_TRUE(calc->has_changed_significantly(50.0, 0.5));
}

// ============================================================================
// Edge Cases
// ============================================================================

void test_calculator_zero_current(void) {
    calc->set_ah(50.0);
    
    // Zero current for 1 hour
    double new_ah = calc->integrate_current(0.0, 3600000);
    
    // Ah should not change
    TEST_ASSERT_EQUAL_FLOAT(50.0f, new_ah);
}

void test_calculator_zero_time(void) {
    calc->set_ah(50.0);
    
    // 10A for 0 milliseconds
    double new_ah = calc->integrate_current(10.0, 0);
    
    // Ah should not change
    TEST_ASSERT_EQUAL_FLOAT(50.0f, new_ah);
}

void test_calculator_very_high_current(void) {
    calc->set_ah(50.0);
    
    // Starter motor: 100A for 3 seconds
    double new_ah = calc->integrate_current(-100.0, 3000);
    
    // 100A * (3000/3600000) hours = 100 * 0.000833 = 0.0833 Ah
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 49.9167f, new_ah);
}

void test_calculator_multiple_integrations(void) {
    calc->set_ah(50.0);
    
    // Multiple small integrations
    for (int i = 0; i < 10; i++) {
        calc->integrate_current(1.0, 360000);  // 1A for 6 minutes = 0.1Ah each
    }
    
    // Should have added 1Ah total
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 51.0f, calc->get_ah());
}

// ============================================================================
// Main Test Runner
// ============================================================================

void setup() {
    delay(2000);  // Wait for serial monitor
    
    UNITY_BEGIN();
    
    // Basic functionality
    RUN_TEST(test_calculator_initialization);
    RUN_TEST(test_calculator_set_ah);
    RUN_TEST(test_calculator_set_charge_efficiency);
    RUN_TEST(test_calculator_set_discharge_efficiency);
    RUN_TEST(test_calculator_capacity_management);
    RUN_TEST(test_calculator_capacity_change_clamps_ah);
    
    // Integration tests
    RUN_TEST(test_calculator_integrate_charging);
    RUN_TEST(test_calculator_integrate_discharging);
    RUN_TEST(test_calculator_integrate_with_charge_efficiency);
    RUN_TEST(test_calculator_integrate_with_discharge_efficiency);
    RUN_TEST(test_calculator_integrate_short_duration);
    RUN_TEST(test_calculator_integrate_clamps_at_full);
    RUN_TEST(test_calculator_integrate_clamps_at_empty);
    
    // SOC tests
    RUN_TEST(test_calculator_soc_at_full);
    RUN_TEST(test_calculator_soc_at_half);
    RUN_TEST(test_calculator_soc_at_empty);
    RUN_TEST(test_calculator_soc_with_degraded_battery);
    RUN_TEST(test_calculator_soc_zero_capacity);
    
    // Change detection
    RUN_TEST(test_calculator_has_changed_significantly);
    
    // Edge cases
    RUN_TEST(test_calculator_zero_current);
    RUN_TEST(test_calculator_zero_time);
    RUN_TEST(test_calculator_very_high_current);
    RUN_TEST(test_calculator_multiple_integrations);
    
    UNITY_END();
}

void loop() {}
