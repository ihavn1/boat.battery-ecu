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
 * - voltage and current (raw sensor readings)
 * - capacity.nominal, capacity.actual, capacity.remaining (joules)
 * - capacity.stateOfCharge (ratio 0-1)
 * 
 * Signal K inputs (PUT requests):
 * - capacity/remaining - Set remaining energy in joules (persists immediately)
 * - capacity/actual - Set actual capacity in joules (persists immediately)
 * - capacity/nominal - Set nominal capacity in joules (persists immediately)
 * - configuration/chargeEfficiency - Custom charge efficiency setting
 * - configuration/dischargeEfficiency - Custom discharge efficiency setting
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

    // Signal K defines battery current as positive out of the battery.
    // The domain model uses positive = charging, so invert at this boundary.
    auto* current_sensor = new RepeatSensor<float>(read_interval, [battery]() { 
        return -battery->current();
    });
    current_sensor->connect_to(
        new SKOutputFloat(config.current_path(), "", new SKMetadata("A", "Amps")));

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

    // Signal K battery capacity is expressed as energy in joules.
    auto* nominal_capacity_sensor = new RepeatSensor<float>(1000, [battery, &config]() {
        return battery->marked_capacity_ah() * config.nominal_voltage() * 3600.0f;
    });
    nominal_capacity_sensor->connect_to(
        new SKOutputFloat(config.nominal_capacity_path(), "",
                  new SKMetadata("J", "Nominal capacity (J; Ah at 12 V)",
                                 "Nominal energy capacity in joules; divide by 43200 to get Ah at 12 V")));

    auto* remaining_capacity_sensor = new RepeatSensor<float>(1000, [battery, &config]() {
        return static_cast<float>(battery->ah() * config.nominal_voltage() * 3600.0);
    });
    remaining_capacity_sensor->connect_to(
        new SKOutputFloat(config.remaining_capacity_path(), "",
                  new SKMetadata("J", "Remaining capacity (J; Ah at 12 V)",
                                 "Remaining energy in joules; divide by 43200 to get Ah at 12 V")));

    if (config.actual_capacity_path() != nullptr) {
      auto* actual_capacity_sensor = new RepeatSensor<float>(1000, [battery, &config]() {
          return battery->current_capacity_ah() * config.nominal_voltage() * 3600.0f;
      });
      actual_capacity_sensor->connect_to(
          new SKOutputFloat(config.actual_capacity_path(), "",
                            new SKMetadata("J", "Actual capacity (J; Ah at 12 V)",
                                           "Actual energy capacity in joules; divide by 43200 to get Ah at 12 V")));
    }
    
    // State of Charge (SOC) as ratio (0-1 for Signal K standard)
    // Battery SOC is calculated as 0-100%, converted to 0-1 ratio here
    auto* soc_sensor = new RepeatSensor<float>(1000, [battery]() { 
        return battery->soc() / 100.0f; 
    });
    soc_sensor->connect_to(
        new SKOutputFloat(config.soc_path(), "", new SKMetadata("ratio", "State of Charge")));

    // Periodic safety-net persistence (hourly, if changed significantly).
    // GPIO27 shutdown handling is now the primary save path (see
    // shutdown_helper.h), triggered right before power is cut.
    sensesp_app->get_event_loop()->onRepeat(3600000, [monitor]() {
        monitor->maybe_persist();
    });
    
    // ========================================================================
    // SIGNAL K INPUTS: Remote configuration via PUT requests
    // ========================================================================
    // Custom ValueConsumer classes handle PUT requests and persist changes
    // immediately to NVS to ensure manual updates are never lost

    // Remaining capacity is received in joules and retained internally in Ah.
    class RemainingCapacityConsumer : public ValueConsumer<float> {
     public:
      RemainingCapacityConsumer(Battery* bat, BatteryMonitor* mon, float nominal_voltage)
          : battery_(bat), monitor_(mon), nominal_voltage_(nominal_voltage) {}
      void set(const float& new_value) override { 
          battery_->set_ah(new_value / (nominal_voltage_ * 3600.0f));
          monitor_->save_state();  // Persist immediately on manual set
      }
     private:
      Battery* battery_;
      BatteryMonitor* monitor_;
      float nominal_voltage_;
    };
    
    auto* remaining_capacity_input =
        new SKPutRequestListener<float>(config.remaining_capacity_path());
    remaining_capacity_input->connect_to(
        new RemainingCapacityConsumer(battery, monitor, config.nominal_voltage()));
    
    // Efficiency settings are custom because Signal K has no standard fields
    // for coulomb-counting efficiency.
    String battery_path = String(config.soc_path());
    battery_path = battery_path.substring(0, battery_path.indexOf(".capacity"));
    String charge_eff_path = battery_path + ".configuration.chargeEfficiency";
    String discharge_eff_path = battery_path + ".configuration.dischargeEfficiency";
    
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
    
        // Actual capacity is received in joules and retained internally in Ah.
        class ActualCapacityConsumer : public ValueConsumer<float> {
     public:
            ActualCapacityConsumer(Battery* bat, BatteryMonitor* mon, float nominal_voltage)
          : battery_(bat), monitor_(mon), nominal_voltage_(nominal_voltage) {}
      void set(const float& new_value) override { 
          battery_->set_current_capacity_ah(new_value / (nominal_voltage_ * 3600.0f));
          monitor_->save_state();  // Persist immediately
      }
     private:
      Battery* battery_;
      BatteryMonitor* monitor_;
      float nominal_voltage_;
    };
    
    // Nominal capacity is received in joules and retained internally in Ah.
    class NominalCapacityConsumer : public ValueConsumer<float> {
     public:
      NominalCapacityConsumer(Battery* bat, BatteryMonitor* mon, float nominal_voltage)
          : battery_(bat), monitor_(mon), nominal_voltage_(nominal_voltage) {}
      void set(const float& new_value) override { 
          battery_->set_marked_capacity_ah(new_value / (nominal_voltage_ * 3600.0f));
          monitor_->save_state();
      }
     private:
      Battery* battery_;
      BatteryMonitor* monitor_;
      float nominal_voltage_;
    };
    
    // Register PUT request listeners for all configuration parameters
    auto* charge_eff_input = new SKPutRequestListener<float>(charge_eff_path);
    charge_eff_input->connect_to(new ChargeEfficiencyConsumer(battery, monitor));
    
    auto* discharge_eff_input = new SKPutRequestListener<float>(discharge_eff_path);
    discharge_eff_input->connect_to(new DischargeEfficiencyConsumer(battery, monitor));
    
    if (config.actual_capacity_path() != nullptr) {
      auto* current_capacity_input = new SKPutRequestListener<float>(config.actual_capacity_path());
      current_capacity_input->connect_to(
          new ActualCapacityConsumer(battery, monitor, config.nominal_voltage()));
    }
    
    auto* marked_capacity_input = new SKPutRequestListener<float>(config.nominal_capacity_path());
    marked_capacity_input->connect_to(
        new NominalCapacityConsumer(battery, monitor, config.nominal_voltage()));

    return monitor;
}

}  // namespace sensesp
