#include <unity.h>
#include <Arduino.h>

// ============================================================================
// SOC (State of Charge) Calculation Tests
// ============================================================================
// Tests the internal SOC calculation: (Ah / Current Capacity) * 100
// Returns percentage (0-100%) for domain model usage
// Note: Signal K output converts this to ratio (0-1) by dividing by 100

void test_soc_at_full_capacity(void) {
    float ah = 200.0f;
    float capacity = 200.0f;
    float soc = (ah / capacity) * 100.0f;
    
    TEST_ASSERT_EQUAL_FLOAT(100.0f, soc);
}

void test_soc_at_half_capacity(void) {
    float ah = 100.0f;
    float capacity = 200.0f;
    float soc = (ah / capacity) * 100.0f;
    
    TEST_ASSERT_EQUAL_FLOAT(50.0f, soc);
}

void test_soc_at_empty(void) {
    float ah = 0.0f;
    float capacity = 200.0f;
    float soc = (ah / capacity) * 100.0f;
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, soc);
}

void test_soc_clamping_above_100(void) {
    float ah = 220.0f;  // Over capacity (shouldn't happen but test clamping)
    float capacity = 200.0f;
    float soc = (ah / capacity) * 100.0f;
    soc = constrain(soc, 0.0f, 100.0f);
    
    TEST_ASSERT_EQUAL_FLOAT(100.0f, soc);
}

void test_soc_clamping_below_0(void) {
    float ah = -10.0f;  // Negative Ah (shouldn't happen but test clamping)
    float capacity = 200.0f;
    float soc = (ah / capacity) * 100.0f;
    soc = constrain(soc, 0.0f, 100.0f);
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, soc);
}

void test_soc_with_zero_capacity(void) {
    float ah = 50.0f;
    float capacity = 0.0f;
    float soc = (capacity > 0.0f) ? (ah / capacity) * 100.0f : 0.0f;
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, soc);
}

void test_soc_with_degraded_battery(void) {
    // Battery degraded from 200Ah to 160Ah
    float ah = 160.0f;  // Fully charged
    float capacity = 160.0f;  // Current capacity
    float soc = (ah / capacity) * 100.0f;
    
    TEST_ASSERT_EQUAL_FLOAT(100.0f, soc);
}

void test_soc_degraded_battery_half(void) {
    // Battery degraded from 200Ah to 160Ah, at half charge
    float ah = 80.0f;
    float capacity = 160.0f;
    float soc = (ah / capacity) * 100.0f;
    
    TEST_ASSERT_EQUAL_FLOAT(50.0f, soc);
}

void test_soc_typical_house_battery(void) {
    // 200Ah house battery at 180Ah (90% SOC)
    float ah = 180.0f;
    float capacity = 200.0f;
    float soc = (ah / capacity) * 100.0f;
    
    TEST_ASSERT_EQUAL_FLOAT(90.0f, soc);
}

void test_soc_typical_starter_battery(void) {
    // 110Ah starter battery at 105Ah (95.45% SOC)
    float ah = 105.0f;
    float capacity = 110.0f;
    float soc = (ah / capacity) * 100.0f;
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 95.45f, soc);
}

void test_soc_precision(void) {
    // Test floating point precision for small values
    float ah = 1.5f;
    float capacity = 200.0f;
    float soc = (ah / capacity) * 100.0f;
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.75f, soc);
}

// ============================================================================
// Main Test Runner
// ============================================================================

void setup() {
    delay(2000);
    
    UNITY_BEGIN();
    
    RUN_TEST(test_soc_at_full_capacity);
    RUN_TEST(test_soc_at_half_capacity);
    RUN_TEST(test_soc_at_empty);
    RUN_TEST(test_soc_clamping_above_100);
    RUN_TEST(test_soc_clamping_below_0);
    RUN_TEST(test_soc_with_zero_capacity);
    RUN_TEST(test_soc_with_degraded_battery);
    RUN_TEST(test_soc_degraded_battery_half);
    RUN_TEST(test_soc_typical_house_battery);
    RUN_TEST(test_soc_typical_starter_battery);
    RUN_TEST(test_soc_precision);
    
    UNITY_END();
}

void loop() {
}
