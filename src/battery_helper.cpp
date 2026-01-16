#include <Arduino.h>
#include "battery_helper.h"
#include "battery.h"
#include "battery_monitor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/signalk/signalk_put_request_listener.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp_app.h"

namespace sensesp {

BatteryMonitor* setupBatterySensor(ISensor& sensor, unsigned int read_interval,
                                   const BatteryConfig& config, IStorageProvider& storage) {
    // Initialize sensor hardware
    if (!sensor.begin()) {
      while (1) {
        delay(10);
      }
    }

    // Create Battery domain object and BatteryMonitor orchestrator
    auto* battery = new Battery(config);
    auto* monitor = new BatteryMonitor(*battery, sensor, storage);

    // Voltage sensor - read and publish to Signal K
    auto* voltage_sensor = new RepeatSensor<float>(read_interval, [battery]() { 
        return battery->voltage(); 
    });
    voltage_sensor->connect_to(
        new SKOutputFloat(config.voltage_path(), "", new SKMetadata("V", "Voltage")));

    // Current sensor - read and publish to Signal K
    auto* current_sensor = new RepeatSensor<float>(read_interval, [battery]() { 
        return battery->current(); 
    });
    current_sensor->connect_to(
        new SKOutputFloat(config.current_path(), "", new SKMetadata("A", "Amps")));

    // Power sensor - read and publish to Signal K
    auto* power_sensor = new RepeatSensor<float>(read_interval, [battery]() { 
        return battery->power(); 
    });
    power_sensor->connect_to(
        new SKOutputFloat(config.power_path(), "", new SKMetadata("W", "Watts")));

    // Update battery readings from sensor at configured interval
    sensesp_app->get_event_loop()->onRepeat(read_interval, [monitor]() {
        monitor->update_readings();
    });

    // Integrate current to update Ah at 1Hz
    sensesp_app->get_event_loop()->onRepeat(1000, [monitor]() {
        monitor->integrate();
    });

    // Amp-hour output - sample and publish to Signal K
    auto* ah_sensor = new RepeatSensor<float>(1000, [battery]() { 
        return battery->ah(); 
    });
    ah_sensor->connect_to(
        new SKOutputFloat(config.ah_path(), "", new SKMetadata("Ah", "Ampere hours")));
    
    // State of Charge percentage output
    auto* soc_sensor = new RepeatSensor<float>(1000, [battery]() { 
        return battery->soc(); 
    });
    soc_sensor->connect_to(
        new SKOutputFloat(config.soc_path(), "", new SKMetadata("ratio", "State of Charge")));

    // Persist state every 10 seconds if changed significantly
    sensesp_app->get_event_loop()->onRepeat(10000, [monitor]() {
        monitor->maybe_persist();
    });
    
    // Signal K input to allow remote reset/calibration of Ah value
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
    
    // Signal K inputs for charge/discharge efficiency configuration
    String charge_eff_path = String(config.ah_path()) + "/chargeEfficiency";
    String discharge_eff_path = String(config.ah_path()) + "/dischargeEfficiency";
    String capacity_path = String(config.ah_path()) + "/capacity";  // Current capacity (degrades)
    String marked_capacity_path = String(config.ah_path()) + "/markedCapacity";  // Nameplate capacity
    
    // Charge efficiency consumer
    class ChargeEfficiencyConsumer : public ValueConsumer<float> {
     public:
      ChargeEfficiencyConsumer(Battery* bat, BatteryMonitor* mon) : battery_(bat), monitor_(mon) {}
      void set(const float& new_value) override { 
          battery_->set_charge_efficiency(new_value);
          monitor_->save_state();
      }
     private:
      Battery* battery_;
      BatteryMonitor* monitor_;
    };
    
    // Discharge efficiency consumer
    class DischargeEfficiencyConsumer : public ValueConsumer<float> {
     public:
      DischargeEfficiencyConsumer(Battery* bat, BatteryMonitor* mon) : battery_(bat), monitor_(mon) {}
      void set(const float& new_value) override { 
          battery_->set_discharge_efficiency(new_value);
          monitor_->save_state();
      }
     private:
      Battery* battery_;
      BatteryMonitor* monitor_;
    };
    
    // Current capacity consumer
    class CurrentCapacityConsumer : public ValueConsumer<float> {
     public:
      CurrentCapacityConsumer(Battery* bat, BatteryMonitor* mon) : battery_(bat), monitor_(mon) {}
      void set(const float& new_value) override { 
          battery_->set_current_capacity_ah(new_value);
          monitor_->save_state();
      }
     private:
      Battery* battery_;
      BatteryMonitor* monitor_;
    };
    
    // Marked capacity consumer (read-only in practice, but exposed for completeness)
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
