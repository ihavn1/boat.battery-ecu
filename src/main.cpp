#include "onewire_helper.h"
#include "battery_helper.h"
// Boilerplate #includes:
#include "sensesp_app_builder.h"
#include "sensesp/signalk/signalk_output.h"

// Sensor-specific #includes:
#include "battery_factory.h"
#include "sensor_factory.h"
#include "storage/nvs_storage_provider.h"
#include "sensesp_onewire/onewire_temperature.h"

using namespace sensesp;

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================
// Use GPIO numbers (not Arduino pin numbers) and named constants to avoid
// magic numbers scattered through the code.

// GPIO pin assignments
static constexpr uint8_t ONEWIRE_PIN = 25;  // Dallas OneWire temperature sensors

// Timing configuration
static constexpr unsigned int TEMPERATURE_READ_DELAY_MS = 2000;  // Temperature read interval
static constexpr unsigned int BATTERY_READ_INTERVAL_MS = 1000;   // Battery sensor read interval

// Battery capacity configuration (in Ah)
static constexpr float HOUSE_BATTERY_CAPACITY_AH = 200.0f;    // House battery nameplate capacity
static constexpr float STARTER_BATTERY_CAPACITY_AH = 110.0f;  // Starter battery nameplate capacity

void setup()
{
    // Initialize logging subsystem
    SetupLogging();

    // Create the global SensESPApp object
    // This initializes WiFi, web server, Signal K client, etc.
    SensESPAppBuilder builder;
    sensesp_app = builder
                      .set_hostname("battery-sensors")
                      ->get_app();

    // Initialize I2C bus for INA226 sensors
    Wire.begin();

    // ========================================================================
    // FACTORY PATTERN: Create hardware instances via factories
    // ========================================================================
    // Factories encapsulate object creation, following Open/Closed Principle.
    // Adding new sensor types (INA219, INA3221) doesn't require modifying
    // existing code.

    // Create INA226 sensor instances (dependency injection)
    // House: I2C 0x40, Starter: I2C 0x41
    ISensor* houseSensor = SensorFactory::createHouseBatterySensor();
    ISensor* starterSensor = SensorFactory::createStarterBatterySensor();

    // Create storage provider for persistent configuration (NVS)
    // Could be swapped for EEPROM or file-based storage via IStorageProvider interface
    NVSStorageProvider storage;

    // Create Battery domain objects using factory
    // BatteryFactory handles BatteryConfig creation with proper Signal K paths
    Battery* houseBattery = BatteryFactory::createHouseBattery(HOUSE_BATTERY_CAPACITY_AH);
    Battery* starterBattery = BatteryFactory::createStarterBattery(STARTER_BATTERY_CAPACITY_AH);

    // ========================================================================
    // BATTERY MONITORING SETUP
    // ========================================================================
    // setupBatterySensor() creates:
    // - BatteryMonitor orchestrator (coordinates sensor, battery, storage)
    // - Signal K outputs (voltage, current, power, Ah, SOC)
    // - Signal K inputs (PUT handlers for Ah, efficiency, capacity)
    // - Event loop reactions (read sensors, integrate current, persist state)

    // House battery: 200Ah, reads voltage/current/power at 1Hz
    setupBatterySensor(*houseSensor, BATTERY_READ_INTERVAL_MS, houseBattery->config(), storage);

    // Starter battery: 110Ah, reads voltage/current/power at 1Hz
    setupBatterySensor(*starterSensor, BATTERY_READ_INTERVAL_MS, starterBattery->config(), storage);

    // ========================================================================
    // TEMPERATURE MONITORING SETUP
    // ========================================================================
    // Dallas OneWire temperature sensors for battery temperature monitoring.
    // TemperatureMonitor class provides calibration and range checking.
    // Signal K paths follow specification:
    // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html

    sensesp::onewire::DallasTemperatureSensors *dts = 
        new sensesp::onewire::DallasTemperatureSensors(ONEWIRE_PIN);

    // House battery temperature sensor
    // Device indices 110, 120, 130 identify specific sensors on the OneWire bus
    add_onewire_temp(dts, TEMPERATURE_READ_DELAY_MS, "houseBatteryTemperature",
                     "electrical.batteries.house.temperature",
                     "House Battery Temperature", 110, 120, 130);

    // Starter battery temperature sensor
    // Device indices 210, 220, 230 identify specific sensors on the OneWire bus
    add_onewire_temp(dts, TEMPERATURE_READ_DELAY_MS, "starterBatteryTemperature",
                     "electrical.batteries.starter.temperature",
                     "Starter Battery Temperature", 210, 220, 230);
}

void loop()
{
    static auto event_loop = sensesp_app->get_event_loop();
    event_loop->tick();
}
