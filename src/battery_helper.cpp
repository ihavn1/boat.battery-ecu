#include <Arduino.h>
#include "battery_helper.h"
#include "battery.h"
#include "battery_monitor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/signalk/signalk_put_request_listener.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp_app.h"

namespace sensesp {

/**
 * @brief Setup battery monitoring pipeline with SOLID architecture
 * 
 * Creates a complete battery monitoring system following SOLID principles:
 * - Battery: Domain model (state + behavior)
 * - BatteryMonitor: Orchestrator (coordinates sensor, storage, domain)
 * - ISensor: Abstraction for hardware (INA226, INA219, etc.)
 * - IStorageProvider: Abstraction for persistence (NVS, EEPROM, etc.)
 * 
 * Architecture layers:
 *   ISensor → BatteryMonitor → Battery → AmpHourCalculator
 *                ↓                ↓
 *         IStorageProvider    Signal K Output
 * 
 * Event loop reactions:
 * - Read sensors at configured interval (e.g., 1Hz)
 * - Integrate current at 1Hz
 * - Persist state every 10s if changed significantly (≥0.5 Ah)
 * 
 * Signal K outputs:
 * - voltage, current, power (raw sensor readings)
 * - ah (integrated amp-hours)
 * - soc (state of charge percentage)
 * 
 * Signal K inputs (PUT requests):
 * - ah - Set Ah value (persists immediately)
 * - ah/chargeEfficiency - Set charge efficiency % (persists immediately)
 * - ah/dischargeEfficiency - Set discharge efficiency % (persists immediately)
 * - ah/capacity - Set current capacity for degraded batteries (persists immediately)
 * - ah/markedCapacity - Set nameplate capacity (for testing/reconfiguration)
 * 
 * @param sensor Sensor implementation (injected dependency)
 * @param read_interval Sensor read interval in milliseconds
 * @param config Battery configuration (paths, capacity, initial state)
 * @param storage Storage provider for persistence (injected dependency)
 * @return BatteryMonitor* pointer for lifecycle management
 */
BatteryMonitor* setupBatterySensor(ISensor& sensor, unsigned int read_interval,
                                   const BatteryConfig& config, IStorageProvider& storage) {
    // Initialize sensor hardware
    // Block until sensor is ready (critical for proper operation)
    if (!sensor.begin()) {
      while (1) {
        delay(10);
      }
    }

    // ========================================================================
    // DEPENDENCY INJECTION: Create domain object and orchestrator
    // ========================================================================
    // Battery: Pure domain model (no framework dependencies)
    // BatteryMonitor: Orchestration layer (coordinates infrastructure)
    auto* battery = new Battery(config);
    auto* monitor = new BatteryMonitor(*battery, sensor, storage);

    // ========================================================================
    // SIGNAL K OUTPUTS: Publish sensor readings
    // ========================================================================
    // RepeatSensor samples values at configured interval and emits to Signal K
    // Lambdas capture battery pointer to read current state
    auto* voltage_sensor = new RepeatSensor<float>(read_interval, [battery]() { 
        return battery->voltage(); 
    });
    voltage_sensor->connect_to(
        new SKOutputFloat(config.voltage_path(), "", new SKMetadata("V", "Voltage")));

    // Current output (A, positive = charging, negative = discharging)
    auto* current_sensor = new RepeatSensor<float>(read_interval, [battery]() { 
        return battery->current(); 
    });
    current_sensor->connect_to(
        new SKOutputFloat(config.current_path(), "", new SKMetadata("A", "Amps")));

    // Power output (W)
    auto* power_sensor = new RepeatSensor<float>(read_interval, [battery]() { 
        return battery->power(); 
    });
    power_sensor->connect_to(
        new SKOutputFloat(config.power_path(), "", new SKMetadata("W", "Watts")));

    // ========================================================================
    // EVENT LOOP REACTIONS: Periodic tasks
    // ========================================================================
    // Use ReactESP event loop for non-blocking periodic execution

    // Update battery readings from sensor hardware
    sensesp_app->get_event_loop()->onRepeat(read_interval, [monitor]() {
        monitor->update_readings();
    });

    // Integrate current to update Ah (1Hz for accuracy)
    sensesp_app->get_event_loop()->onRepeat(1000, [monitor]() {
        monitor->integrate();
    });

    // ========================================================================
    // SIGNAL K OUTPUTS: Battery state
    // ========================================================================

    // Amp-hour (Ah) output - integrated current over time
    auto* ah_sensor = new RepeatSensor<float>(1000, [battery]() { 
        return battery->ah(); 
    });
    ah_sensor->connect_to(
        new SKOutputFloat(config.ah_path(), "", new SKMetadata("Ah", "Ampere hours")));
    
    // State of Charge (SOC) percentage output (0-100%)
    auto* soc_sensor = new RepeatSensor<float>(1000, [battery]() { 
        return battery->soc(); 
    });
    soc_sensor->connect_to(
        new SKOutputFloat(config.soc_path(), "", new SKMetadata("ratio", "State of Charge")));

    // Persist state every 10 seconds if changed significantly (≥0.5 Ah)
    sensesp_app->get_event_loop()->onRepeat(10000, [monitor]() {
        monitor->maybe_persist();
    });
    
    // ========================================================================
    // SIGNAL K INPUTS: Remote configuration via PUT requests
    // ========================================================================
    // Custom ValueConsumer classes handle PUT requests and persist changes
    // immediately to NVS to ensure manual updates are never lost

    // Ah value consumer - sets Ah and persists immediately
    class AhConsumer : public ValueConsumer<float> {
     public:
      AhConsumer(Battery* bat, BatteryMonitor* mon) : battery_(bat), monitor_(mon) {}
      void set(const float& new_value) override { 
          battery_->set_ah(new_value);
          monitor_->save_state();  // Persist immediately on manual set
      }
     private:
      Battery* battery_;
      BatteryMonitor* monitor_;
    };
    
    auto* ah_sk_input = new SKPutRequestListener<float>(config.ah_path());
    ah_sk_input->connect_to(new AhConsumer(battery, monitor));
    
    // Configuration path structure: base_path/chargeEfficiency, etc.
    String charge_eff_path = String(config.ah_path()) + "/chargeEfficiency";
    String discharge_eff_path = String(config.ah_path()) + "/dischargeEfficiency";
    String capacity_path = String(config.ah_path()) + "/capacity";  // Current capacity (degrades)
    String marked_capacity_path = String(config.ah_path()) + "/markedCapacity";  // Nameplate capacity
    
    // Charge efficiency consumer (0-100%, affects charging)
    class ChargeEfficiencyConsumer : public ValueConsumer<float> {
     public:
      ChargeEfficiencyConsumer(Battery* bat, BatteryMonitor* mon) : battery_(bat), monitor_(mon) {}
      void set(const float& new_value) override { 
          battery_->set_charge_efficiency(new_value);
          monitor_->save_state();  // Persist immediately
      }
     private:
      Battery* battery_;
      BatteryMonitor* monitor_;
    };
    
    // Discharge efficiency consumer (0-100%, affects discharging)
    class DischargeEfficiencyConsumer : public ValueConsumer<float> {
     public:
      DischargeEfficiencyConsumer(Battery* bat, BatteryMonitor* mon) : battery_(bat), monitor_(mon) {}
      void set(const float& new_value) override { 
          battery_->set_discharge_efficiency(new_value);
          monitor_->save_state();  // Persist immediately
      }
     private:
      Battery* battery_;
      BatteryMonitor* monitor_;
    };
    
    // Current capacity consumer (for degraded batteries)
    class CurrentCapacityConsumer : public ValueConsumer<float> {
     public:
      CurrentCapacityConsumer(Battery* bat, BatteryMonitor* mon) : battery_(bat), monitor_(mon) {}
      void set(const float& new_value) override { 
          battery_->set_current_capacity_ah(new_value);
          monitor_->save_state();  // Persist immediately
      }
     private:
      Battery* battery_;
      BatteryMonitor* monitor_;
    };
    
    // Marked capacity consumer (nameplate rating, rarely changes)
    class MarkedCapacityConsumer : public ValueConsumer<float> {
     public:
      MarkedCapacityConsumer(Battery* bat) : battery_(bat) {}
      void set(const float& new_value) override { 
          // Marked capacity typically doesn't change, but allow it for testing
          battery_->set_marked_capacity_ah(new_value);
      }
     private:
      Battery* battery_;
    };
    
    // Register PUT request listeners for all configuration parameters
    auto* charge_eff_input = new SKPutRequestListener<float>(charge_eff_path);
    charge_eff_input->connect_to(new ChargeEfficiencyConsumer(battery, monitor));
    
    auto* discharge_eff_input = new SKPutRequestListener<float>(discharge_eff_path);
    discharge_eff_input->connect_to(new DischargeEfficiencyConsumer(battery, monitor));
    
    auto* current_capacity_input = new SKPutRequestListener<float>(capacity_path);
    current_capacity_input->connect_to(new CurrentCapacityConsumer(battery, monitor));
    
    auto* marked_capacity_input = new SKPutRequestListener<float>(marked_capacity_path);
    marked_capacity_input->connect_to(new MarkedCapacityConsumer(battery));

    return monitor;
}

}  // namespace sensesp
