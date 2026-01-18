# Changelog

## 2026-01-18 - Signal K SOC Format Correction

### Changed
- **SOC Output**: SOC is now correctly sent to Signal K as a ratio (0-1) per Signal K specification
  - Internal calculation still returns percentage (0-100%) for domain logic
  - Conversion `battery->soc() / 100.0f` applied in Signal K output layer
  - Updated all documentation to reflect this distinction
  - Test procedures now expect ratio values (e.g., 0.5 instead of 50%)

### Documentation Updates
- README.md: Clarified SOC sent as ratio to Signal K
- .github/copilot-instructions.md: Added SOC conversion example in data pipeline
- MANUAL_TEST_PROCEDURES.md: Updated expected values to ratios
- include/battery.h: Added comments clarifying percentage vs ratio
- include/ah_calculator.h: Documented that calculate_soc() returns percentage
- test/test_battery_helper/test_soc_calculation.cpp: Added note about Signal K conversion

## 2026-01-16 - SOLID Architecture Refactoring

### Major Changes
- **SOLID Principles**: Complete refactoring following Dependency Inversion, Single Responsibility, Open/Closed, Liskov Substitution, and Interface Segregation principles
- **Domain-Driven Design**: Separated domain logic (Battery, AmpHourCalculator) from infrastructure (sensors, storage)
- **Factory Patterns**: Added BatteryFactory and SensorFactory for object creation
- **Orchestration Layer**: Created BatteryMonitor and TemperatureMonitor for coordinating infrastructure and domain

### New Classes
- `Battery` - Pure domain model with state and behavior (no framework dependencies)
- `BatteryMonitor` - Orchestrates sensor reading, integration, and persistence
- `TemperatureMonitor` - Temperature sensor orchestration with calibration
- `BatteryFactory` - Creates Battery instances (house, starter, custom)
- `SensorFactory` - Creates sensor instances (INA226, extensible for INA219, etc.)
- `ISensor` - Sensor abstraction interface
- `ITemperatureSensor` - Temperature sensor abstraction
- `IStorageProvider` - Storage abstraction (NVS, EEPROM, file system)

### Architecture Changes
- Moved from direct AmpHourIntegrator usage to Battery + BatteryMonitor pattern
- All hardware dependencies now injected via interfaces
- Clean separation: Domain → Orchestration → Infrastructure
- Event loop reactions replace manual timer management

### Testing
- Added 45 new tests (Battery: 23, BatteryMonitor: 9, TemperatureMonitor: 13)
- Total: 171 tests, 100% passing
- Comprehensive mock-based unit testing
- Real-world scenario tests included

### Code Quality
- Removed dead global INA226 instances from main.cpp
- Fixed critical bug: Static INA226 factory would reuse first instance for all sensors
- Eliminated redundant variable assignments
- Removed unused includes
- Cleaned up inline comments
- All 171 tests passing

### Documentation
- Updated README.md with SOLID architecture overview
- Updated inline comments to reflect new architecture
- Documented factory patterns and dependency injection
- Added architecture diagrams and data flow

### Breaking Changes
- None - API remains compatible with existing Signal K paths
- Internal refactoring only, external behavior unchanged

### Notes
- `ah_integrator.cpp/h` deleted (replaced by Battery + AmpHourCalculator)
- All persistence still uses NVS with same keys
- Signal K paths unchanged
- Remote configuration via PUT requests unchanged

---

## 2025-11-30 - Immediate Ah Persistence

- Persist Amp-hour (Ah) immediately when receiving an SK PUT for Ah. This complements
  the existing periodic/delta-based persistence to ensure manual resets/updates
  are stored to NVS immediately.
- Removed all remaining Serial debug output from `src/ah_integrator.cpp` and
  `src/battery_helper.cpp`.

Notes:
- Ah is still periodically persisted (every 10 minutes by default) if it
  changes by at least 0.5 Ah to reduce NVS wear.
