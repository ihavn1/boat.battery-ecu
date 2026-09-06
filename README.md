# ESP32 Battery Monitor (SensESP-based) - SOLID Architecture

ESP32-based battery monitoring system for marine vessels, measuring voltage, current, power, temperature, and tracking amp-hours (Ah) for house (200Ah) and starter (110Ah) batteries. Integrates with Signal K server for marine data distribution with remote configuration via Signal K PUT requests.

## Architecture Overview

This project follows **SOLID principles** for maintainability, testability, and extensibility:

### Domain Layer (Pure Business Logic)
- **Battery** (`include/battery.h`) - Core battery domain model with state and behavior
- **AmpHourCalculator** (`include/ah_calculator.h`) - Pure calculation logic for Ah integration
- **BatteryConfig** (`include/battery_config.h`) - Battery configuration data structure

### Orchestration Layer (Application Services)
- **BatteryMonitor** (`include/battery_monitor.h`) - Coordinates sensor reading, integration, and persistence
- **TemperatureMonitor** (`include/temperature_monitor.h`) - Temperature sensor orchestration with calibration
- **ShutdownCoordinator** (`include/shutdown_coordinator.h`) - Debounces a GPIO27 low signal, persists both batteries, then requests deep sleep

### Infrastructure Layer (Interfaces & Implementations)
- **ISensor** (`include/sensors/i_sensor.h`) - Sensor abstraction (voltage/current/power)
- **ITemperatureSensor** (`include/sensors/i_temperature_sensor.h`) - Temperature sensor abstraction
- **IStorageProvider** (`include/storage/i_storage_provider.h`) - Persistence abstraction
- **INA226Sensor** (`include/sensors/ina226_sensor.h`) - INA226 hardware implementation
- **NVSStorageProvider** (`include/storage/nvs_storage_provider.h`) - ESP32 NVS storage
- **IClock / ISleepController** (`include/shutdown_interfaces.h`) - Abstractions for the shutdown clock and deep-sleep trigger
- **ShutdownMonitor** (`include/shutdown_monitor.h`) - ISR + FreeRTOS task watching GPIO27

### Factory Layer (Object Creation)
- **BatteryFactory** (`include/battery_factory.h`) - Creates Battery instances from configuration
- **SensorFactory** (`include/sensor_factory.h`) - Creates sensor instances (INA226, etc.)

## Hardware

- **MCU**: AZ-Delivery ESP32 Dev Kit C V4
- **Current/Voltage Sensors**: 2x INA226 (I2C addresses 0x40 house, 0x41 starter)
- **Temperature Sensors**: Dallas OneWire (GPIO 25)

## Key Dependencies

- SensESP 3.1.1 (Signal K integration)
- INA226 library 0.6.5
- OneWire 3.0.2
- Preferences 2.0.0 (NVS storage)

## Data Flow

```
INA226 Hardware → ISensor → BatteryMonitor → Battery (domain)
                                  ↓              ↓
                           IStorageProvider   AmpHourCalculator
                                  ↓              ↓
                                 NVS        SOC Calculation
                                              ↓
                                        Signal K Output
```

## Features

### Battery Monitoring
- ✅ Voltage, current, power measurement at 1Hz
- ✅ Amp-hour (Ah) integration with efficiency compensation
- ✅ State of Charge (SOC) calculation (sent to Signal K as 0-1 ratio)
- ✅ Battery health tracking (capacity degradation)
- ✅ Persistent state across reboots (NVS storage)
- ✅ Emergency persistence on power loss: a low signal on GPIO27 saves both
  batteries and enters deep sleep before the supply voltage is cut
- ✅ Hourly safety-net persistence (in addition to the GPIO27 path and
  immediate PUT-triggered saves)

### Remote Configuration (Signal K PUT)
- Set Ah value manually
- Configure charge/discharge efficiency (0-100%)
- Adjust current capacity (for battery degradation)
- All changes persist immediately to NVS

### Temperature Monitoring
- Dallas OneWire sensors with linear calibration
- Temperature range validation (normal/critically hot/cold)
- Per-battery temperature tracking

## Development

### Build & Deploy
```bash
# Build for ESP32
pio run -e az-delivery-devkit-v4

# Upload to device
pio run -e az-delivery-devkit-v4 --target upload

# Monitor serial output (115200 baud)
pio device monitor

# Clean build artifacts
pio run --target clean
```

### Testing
```bash
# Run all unit tests (177 tests)
pio test -e az-delivery-devkit-v4

# Run specific test suite
pio test -e az-delivery-devkit-v4 --filter test_battery
```

### Test Coverage
- **Domain Layer**: Battery (23 tests), AmpHourCalculator (23 tests), BatteryConfig (11 tests)
- **Orchestration**: BatteryMonitor (9 tests), TemperatureMonitor (13 tests), ShutdownCoordinator (6 tests)
- **Infrastructure**: ISensor (10 tests), ITemperatureSensor (12 tests), IStorageProvider (15 tests)
- **Integration**: SOC calculation (11 tests), Signal K paths (26 tests), INA226 config (12 tests)
- **Total**: 177 tests, 100% passing

## Signal K Integration

### Published Paths
- `electrical.batteries.house.voltage` - House battery voltage (V)
- `electrical.batteries.house.current` - House battery current (A, + out of battery, - into battery)
- `electrical.batteries.house.temperature` - House battery temperature (K)
- `electrical.batteries.house.capacity.nominal` - Nameplate energy capacity (J)
- `electrical.batteries.house.capacity.actual` - Current usable energy capacity (J)
- `electrical.batteries.house.capacity.remaining` - Remaining energy (J)
- `electrical.batteries.house.capacity.stateOfCharge` - House battery SOC (ratio 0-1)
- (Similar paths for `starter` battery)

Capacity values remain in the official Signal K unit, joules. The firmware
metadata identifies the equivalent amp-hour interpretation at the configured
12 V nominal voltage: `Ah = J / 43200`.

### Configuration Paths (PUT requests)
- `electrical.batteries.{house|starter}.capacity.remaining` - Set remaining energy (J)
- `electrical.batteries.{house|starter}.capacity.actual` - Set current usable energy capacity (J)
- `electrical.batteries.{house|starter}.capacity.nominal` - Set nameplate energy capacity (J)
- `electrical.batteries.{house|starter}.configuration.chargeEfficiency` - Custom charge efficiency (%)
- `electrical.batteries.{house|starter}.configuration.dischargeEfficiency` - Custom discharge efficiency (%)

All five parameters persist to NVS immediately when set via PUT.

See [NODE_RED_CONFIGURATION.md](NODE_RED_CONFIGURATION.md) for examples.

## Configuration

### Battery Parameters (main.cpp)
```cpp
static constexpr float HOUSE_BATTERY_CAPACITY_AH = 200.0f;
static constexpr float STARTER_BATTERY_CAPACITY_AH = 110.0f;
static constexpr unsigned int BATTERY_READ_INTERVAL_MS = 1000;
```

### Emergency Shutdown (main.cpp)
```cpp
static constexpr uint8_t SHUTDOWN_PIN = 27;          // Active low, external voltage divider
static constexpr uint64_t SHUTDOWN_DEBOUNCE_MS = 0;  // Min. time SHUTDOWN_PIN must stay low
```
GPIO27 uses plain `INPUT` (no internal pull-up) since the signal level is
driven externally.

### Sensor Configuration (sensor_factory.cpp)
- House battery: INA226 @ 0x40
- Starter battery: INA226 @ 0x41
- Shunt resistance: 0.0075Ω
- Current LSB: 0.250mA
- Averaging: 256 samples

### Temperature Sensors (main.cpp)
- GPIO pin: 25
- Read interval: 2000ms
- Linear calibration support via TemperatureMonitor

## SOLID Principles Applied

### Dependency Inversion
- All components depend on abstractions (ISensor, IStorageProvider, etc.)
- Hardware implementations are injected via factories
- Easy to mock for testing and swap implementations

### Single Responsibility
- Battery: Domain model (state + behavior)
- BatteryMonitor: Orchestration (coordinates infrastructure and domain)
- AmpHourCalculator: Pure calculation logic
- Each class has one reason to change

### Open/Closed
- Factory patterns allow adding new sensor types without modifying existing code
- New battery types can be added via BatteryFactory
- Extensible for INA219, INA3221, ACS712, etc.

### Liskov Substitution
- All ISensor implementations are interchangeable
- IStorageProvider can be swapped (NVS → EEPROM → File system)

### Interface Segregation
- Focused interfaces (ISensor, ITemperatureSensor, IStorageProvider)
- Clients only depend on methods they use

## Project Structure

```
include/
  battery.h                    # Battery domain model
  battery_config.h             # Configuration data
  battery_factory.h            # Battery creation factory
  battery_helper.h             # Setup helper functions
  battery_monitor.h            # Battery orchestration
  temperature_monitor.h        # Temperature orchestration
  ah_calculator.h              # Ah integration logic
  shutdown_interfaces.h        # IClock / ISleepController abstractions
  shutdown_coordinator.h       # GPIO27 debounce + persist + sleep logic
  shutdown_monitor.h           # GPIO27 ISR + FreeRTOS task declaration
  shutdown_helper.h            # Shutdown wiring declaration
  sensors/
    i_sensor.h                 # Sensor interface
    i_temperature_sensor.h     # Temperature sensor interface
    ina226_sensor.h            # INA226 implementation
  storage/
    i_storage_provider.h       # Storage interface
    nvs_storage_provider.h     # NVS implementation
  sensor_factory.h             # Sensor creation factory

src/
  main.cpp                     # Application entry point
  battery_helper.cpp           # Setup implementation
  sensor_factory.cpp           # Factory static members
  onewire_helper.cpp           # Temperature sensor setup
  shutdown_monitor.cpp         # GPIO27 ISR + FreeRTOS task
  shutdown_helper.cpp          # Shutdown wiring (clock, sleep controller)

test/
  test_battery/                # Battery domain tests (23)
  test_battery_monitor/        # BatteryMonitor tests (9)
  test_temperature_monitor/    # TemperatureMonitor tests (13)
  test_calculator/             # AmpHourCalculator tests (23)
  test_sensor_interface/       # ISensor tests (10)
  test_storage_interface/      # IStorageProvider tests (15)
  test_shutdown_coordinator/   # ShutdownCoordinator tests (6)
  (... 13 test suites total, 177 tests)
```

## Contributing

When adding features:
1. Write tests first (TDD)
2. Follow SOLID principles
3. Use factories for object creation
4. Keep domain logic pure (no framework dependencies)
5. Document Signal K paths in NODE_RED_CONFIGURATION.md

## Documentation

- [CHANGELOG.md](CHANGELOG.md) - Version history and changes
- [NODE_RED_CONFIGURATION.md](NODE_RED_CONFIGURATION.md) - Signal K PUT request examples
- [MANUAL_TEST_PROCEDURES.md](MANUAL_TEST_PROCEDURES.md) - Manual test procedures
- [.github/copilot-instructions.md](.github/copilot-instructions.md) - AI assistant context

## License

MIT
