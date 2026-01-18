#pragma once

#include <Arduino.h>

// Pure calculation logic for amp-hour integration - no framework dependencies
// This class contains the core business logic and can be unit tested
class AmpHourCalculator {
 public:
  explicit AmpHourCalculator(float initial_ah = 0.0f, float capacity_ah = 0.0f)
      : ah_(initial_ah),
        capacity_ah_(capacity_ah),
        marked_capacity_ah_(capacity_ah),
        charge_efficiency_(100.0f),
        discharge_efficiency_(100.0f) {}

  // Core calculation: integrate current over time
  // Returns new Ah value after integration
  double integrate_current(double current_a, unsigned long dt_ms) {
    double dt_hours = static_cast<double>(dt_ms) / 3600000.0;
    double delta_ah = current_a * dt_hours;
    
    // Apply efficiency based on current direction
    double efficiency_factor = (current_a > 0) ? (charge_efficiency_ / 100.0) : (discharge_efficiency_ / 100.0);
    delta_ah *= efficiency_factor;
    
    ah_ += delta_ah;
    
    // Clamp to capacity
    if (capacity_ah_ > 0) {
      ah_ = constrain(ah_, 0.0, static_cast<double>(capacity_ah_));
    }
    
    return ah_;
  }

  // Getters
  double get_ah() const { return ah_; }
  float get_charge_efficiency() const { return charge_efficiency_; }
  float get_discharge_efficiency() const { return discharge_efficiency_; }
  float get_current_capacity_ah() const { return capacity_ah_; }
  float get_marked_capacity_ah() const { return marked_capacity_ah_; }

  // Setters with validation
  void set_ah(double ah) {
    if (capacity_ah_ > 0) {
      ah_ = constrain(ah, 0.0, static_cast<double>(capacity_ah_));
    } else {
      ah_ = ah;
    }
  }

  void set_charge_efficiency(float pct) {
    charge_efficiency_ = constrain(pct, 0.0f, 100.0f);
  }

  void set_discharge_efficiency(float pct) {
    discharge_efficiency_ = constrain(pct, 0.0f, 100.0f);
  }

  void set_current_capacity_ah(float capacity) {
    capacity_ah_ = constrain(capacity, 0.1f, 10000.0f);
    // Re-clamp Ah to new capacity
    if (ah_ > capacity_ah_) {
      ah_ = capacity_ah_;
    }
  }

  void set_marked_capacity_ah(float capacity) {
    marked_capacity_ah_ = constrain(capacity, 0.1f, 10000.0f);
  }

  // SOC calculation - returns percentage (0-100%)
  // For Signal K ratio (0-1), divide result by 100
  float calculate_soc() const {
    if (capacity_ah_ <= 0) return 0.0f;
    float soc = (static_cast<float>(ah_) / capacity_ah_) * 100.0f;
    return constrain(soc, 0.0f, 100.0f);
  }

  // Check if Ah has changed significantly since last value
  bool has_changed_significantly(double last_value, double threshold) const {
    return fabs(ah_ - last_value) >= threshold;
  }

 private:
  double ah_;
  float capacity_ah_;
  float marked_capacity_ah_;
  float charge_efficiency_;
  float discharge_efficiency_;
};
