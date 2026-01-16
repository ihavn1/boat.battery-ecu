#pragma once

#include "i_storage_provider.h"
#include <Preferences.h>

namespace sensesp {

/**
 * @brief ESP32 NVS (Non-Volatile Storage) implementation
 * 
 * Wraps ESP32's Preferences library to implement IStorageProvider interface.
 * This allows the system to swap NVS for other storage backends
 * without changing dependent code.
 */
class NVSStorageProvider : public IStorageProvider {
 public:
  NVSStorageProvider() = default;

  bool begin(const char* namespace_name, bool readonly = false) override {
    return prefs_.begin(namespace_name, readonly);
  }

  void end() override {
    prefs_.end();
  }

  bool isKey(const char* key) override {
    return prefs_.isKey(key);
  }

  float getFloat(const char* key, float default_value = 0.0f) override {
    return prefs_.getFloat(key, default_value);
  }

  size_t putFloat(const char* key, float value) override {
    return prefs_.putFloat(key, value);
  }

  String getString(const char* key, const String& default_value = "") override {
    return prefs_.getString(key, default_value);
  }

  size_t putString(const char* key, const String& value) override {
    return prefs_.putString(key, value);
  }

  bool remove(const char* key) override {
    return prefs_.remove(key);
  }

  bool clear() override {
    return prefs_.clear();
  }

 private:
  Preferences prefs_;
};

}  // namespace sensesp
