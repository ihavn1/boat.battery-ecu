#pragma once

#include <Arduino.h>

namespace sensesp {

/**
 * @brief Interface for persistent key-value storage
 * 
 * Abstracts storage implementation to allow different backends
 * (NVS, EEPROM, file system, in-memory for testing) to be used
 * interchangeably. Follows the Dependency Inversion Principle.
 */
class IStorageProvider {
 public:
  virtual ~IStorageProvider() = default;

  /**
   * @brief Open a namespace for reading/writing
   * @param namespace_name Namespace to open (e.g., "battcfg")
   * @param readonly true for read-only access, false for read-write
   * @return true if namespace opened successfully
   */
  virtual bool begin(const char* namespace_name, bool readonly = false) = 0;

  /**
   * @brief Close the current namespace
   */
  virtual void end() = 0;

  /**
   * @brief Check if a key exists in current namespace
   * @param key Key name to check
   * @return true if key exists
   */
  virtual bool isKey(const char* key) = 0;

  /**
   * @brief Read float value from storage
   * @param key Key name
   * @param default_value Value to return if key doesn't exist
   * @return Stored value or default_value
   */
  virtual float getFloat(const char* key, float default_value = 0.0f) = 0;

  /**
   * @brief Write float value to storage
   * @param key Key name (max 15 chars for ESP32 NVS)
   * @param value Value to store
   * @return Number of bytes written, 0 on error
   */
  virtual size_t putFloat(const char* key, float value) = 0;

  /**
   * @brief Read string value from storage
   * @param key Key name
   * @param default_value Value to return if key doesn't exist
   * @return Stored value or default_value
   */
  virtual String getString(const char* key, const String& default_value = "") = 0;

  /**
   * @brief Write string value to storage
   * @param key Key name
   * @param value Value to store
   * @return Number of bytes written, 0 on error
   */
  virtual size_t putString(const char* key, const String& value) = 0;

  /**
   * @brief Remove a key from storage
   * @param key Key name to remove
   * @return true if successful
   */
  virtual bool remove(const char* key) = 0;

  /**
   * @brief Clear all keys in current namespace
   * @return true if successful
   */
  virtual bool clear() = 0;
};

}  // namespace sensesp
