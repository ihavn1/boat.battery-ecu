#include "ah_integrator.h"
#include <Arduino.h>
#include <cmath>

namespace sensesp {

AmpHourIntegrator::AmpHourIntegrator(const String& config_path, float initial_ah, 
                                     float battery_capacity_ah, IStorageProvider& storage)
    : FloatTransform(config_path),
      calculator_(initial_ah, battery_capacity_ah),
      config_path_(config_path),
      storage_(storage) {
  this->output_ = initial_ah;
  last_update_ms_ = millis();
  last_persisted_ah_ = initial_ah;

  // Load persisted state from storage
  if (config_path_.length() > 0) {
    String key = config_path_;
    key.replace('/', '_');
    if (storage_.begin("battcfg", false)) {
      // Load capacities
      if (storage_.isKey((key + "_marked").c_str())) {
        calculator_.set_marked_capacity_ah(storage_.getFloat((key + "_marked").c_str(), battery_capacity_ah));
      }
      if (storage_.isKey((key + "_current").c_str())) {
        calculator_.set_current_capacity_ah(storage_.getFloat((key + "_current").c_str(), battery_capacity_ah));
      }
      // Load efficiencies
      if (storage_.isKey((key + "_charge").c_str())) {
        calculator_.set_charge_efficiency(storage_.getFloat((key + "_charge").c_str(), 100.0f));
      }
      if (storage_.isKey((key + "_discharge").c_str())) {
        calculator_.set_discharge_efficiency(storage_.getFloat((key + "_discharge").c_str(), 100.0f));
      }
      // Load Ah value
      if (storage_.isKey((key + "_ah").c_str())) {
        float v = storage_.getFloat((key + "_ah").c_str(), initial_ah);
        calculator_.set_ah(v);
        this->output_ = v;
        last_persisted_ah_ = v;
      }
      storage_.end();
    }
  }

  // Start timers
  event_loop()->onRepeat(AH_INTEGRATION_INTERVAL_MS, [this]() { this->integrate(); });
  event_loop()->onRepeat(AH_PERSIST_CHECK_INTERVAL_MS, [this]() { this->maybe_persist_ah(); });
}

void AmpHourIntegrator::set(const float& new_value) {
  current_a_ = new_value;
}

void AmpHourIntegrator::set_ah(double ah) {
  calculator_.set_ah(ah);
  this->output_ = calculator_.get_ah();
  ah_dirty_ = true;

  // Persist immediately (SK PUT request)
  if (config_path_.length() > 0) {
    String key = config_path_;
    key.replace('/', '_');
    if (storage_.begin("battcfg", false)) {
      storage_.putFloat((key + "_ah").c_str(), (float)calculator_.get_ah());
      last_persisted_ah_ = calculator_.get_ah();
      last_ah_persist_ms_ = millis();
      ah_dirty_ = false;
      storage_.end();
    }
  }
}

void AmpHourIntegrator::maybe_persist_ah() {
  if (!ah_dirty_ || config_path_.length() == 0) {
    return;
  }
  unsigned long now = millis();
  if (now - last_ah_persist_ms_ < ah_persist_interval_ms_) {
    return;
  }
  
  // Check if changed significantly
  if (!calculator_.has_changed_significantly(last_persisted_ah_, ah_persist_delta_)) {
    ah_dirty_ = false;
    return;
  }

  String key = config_path_;
  key.replace('/', '_');
  if (storage_.begin("battcfg", false)) {
    storage_.putFloat((key + "_ah").c_str(), (float)calculator_.get_ah());
    last_persisted_ah_ = calculator_.get_ah();
    last_ah_persist_ms_ = now;
    ah_dirty_ = false;
    storage_.end();
  }
}

void AmpHourIntegrator::set_marked_capacity_ah(float capacity_ah) {
  calculator_.set_marked_capacity_ah(capacity_ah);
  if (config_path_.length() > 0) {
    String key = config_path_;
    key.replace('/', '_');
    if (storage_.begin("battcfg", false)) {
      storage_.putFloat((key + "_marked").c_str(), calculator_.get_marked_capacity_ah());
      storage_.end();
    }
  }
}

void AmpHourIntegrator::set_current_capacity_ah(float capacity_ah) {
  calculator_.set_current_capacity_ah(capacity_ah);
  if (config_path_.length() > 0) {
    String key = config_path_;
    key.replace('/', '_');
    if (storage_.begin("battcfg", false)) {
      storage_.putFloat((key + "_current").c_str(), calculator_.get_current_capacity_ah());
      storage_.end();
    }
  }
}

void AmpHourIntegrator::set_charge_efficiency(float pct) {
  calculator_.set_charge_efficiency(pct);
  if (config_path_.length() > 0) {
    String key = config_path_;
    key.replace('/', '_');
    if (storage_.begin("battcfg", false)) {
      storage_.putFloat((key + "_charge").c_str(), calculator_.get_charge_efficiency());
      storage_.end();
    }
  }
}

void AmpHourIntegrator::set_discharge_efficiency(float pct) {
  calculator_.set_discharge_efficiency(pct);
  if (config_path_.length() > 0) {
    String key = config_path_;
    key.replace('/', '_');
    if (storage_.begin("battcfg", false)) {
      storage_.putFloat((key + "_discharge").c_str(), calculator_.get_discharge_efficiency());
      storage_.end();
    }
  }
}

void AmpHourIntegrator::integrate() {
  unsigned long now = millis();
  unsigned long dt_ms = now - last_update_ms_;
  last_update_ms_ = now;

  // Delegate to calculator (tested logic)
  calculator_.integrate_current(current_a_, dt_ms);
  this->output_ = calculator_.get_ah();
  
  // Mark dirty for periodic persistence
  if (calculator_.has_changed_significantly(last_persisted_ah_, ah_persist_delta_)) {
    ah_dirty_ = true;
  }
}

}  // namespace sensesp
