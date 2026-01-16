#pragma once

#include "sensesp/transforms/transform.h"
#include "sensesp_base_app.h"
#include "ah_calculator.h"
#include "storage/i_storage_provider.h"

namespace sensesp {

// Interval configuration (can be overridden at compile time)
// Integration interval in milliseconds (default 1 Hz)
#ifndef AH_INTEGRATION_INTERVAL_MS
#define AH_INTEGRATION_INTERVAL_MS 1000
#endif

// Persist-check interval in milliseconds (default 0.2 Hz -> every 5 seconds)
#ifndef AH_PERSIST_CHECK_INTERVAL_MS
#define AH_PERSIST_CHECK_INTERVAL_MS 5000
#endif


// Integrates current (A) over time to produce Amp-hours (Ah).
// Runs internal integration at a configurable interval (default 1 Hz via AH_INTEGRATION_INTERVAL_MS).
// Exposes Ah to consumers at their own polling rate (e.g., Signal K output).
class AmpHourIntegrator : public FloatTransform {
 public:
  // config_path: NVS key prefix for persisting state
  // initial_ah: Starting Ah value
  // battery_capacity_ah: Capacity in Ah, used to clamp Ah between 0 and capacity
  // storage: Storage backend for persisting configuration and state
  explicit AmpHourIntegrator(const String& config_path, float initial_ah, 
                             float battery_capacity_ah,
                             IStorageProvider& storage);

  void set(const float& new_value) override;

  double get_ah() const { return calculator_.get_ah(); }
  void set_ah(double ah);
  
  float get_charge_efficiency() const { return calculator_.get_charge_efficiency(); }
  void set_charge_efficiency(float pct);
  
  float get_discharge_efficiency() const { return calculator_.get_discharge_efficiency(); }
  void set_discharge_efficiency(float pct);
  
  float get_marked_capacity_ah() const { return calculator_.get_marked_capacity_ah(); }
  void set_marked_capacity_ah(float capacity_ah);

  float get_current_capacity_ah() const { return calculator_.get_current_capacity_ah(); }
  void set_current_capacity_ah(float capacity_ah);

 private:
  void integrate();  // Called by internal timer (interval set by AH_INTEGRATION_INTERVAL_MS)
  
  // Core calculation logic (tested independently)
  AmpHourCalculator calculator_;
  
  // SensESP-specific state
  unsigned long last_update_ms_ = 0;
  double current_a_ = 0.0;  // Most recent current reading (A)
  String config_path_;      // NVS key prefix
  IStorageProvider& storage_;  // Storage backend
  
  // Persistence tracking
  bool ah_dirty_ = false;
  unsigned long last_ah_persist_ms_ = 0;
  double last_persisted_ah_ = 0.0;
  unsigned long ah_persist_interval_ms_ = 600000; // 10 minutes
  double ah_persist_delta_ = 0.5;
  void maybe_persist_ah();
};

}  // namespace sensesp
