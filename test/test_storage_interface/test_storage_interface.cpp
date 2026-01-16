#include <unity.h>
#include <Arduino.h>
#include <map>
#include "storage/i_storage_provider.h"

using namespace sensesp;

// Mock storage provider for testing
class MockStorageProvider : public IStorageProvider {
 public:
  MockStorageProvider() : is_open_(false), readonly_(false) {}

  bool begin(const char* namespace_name, bool readonly = false) override {
    if (is_open_) {
      return false;  // Already open
    }
    current_namespace_ = namespace_name;
    readonly_ = readonly;
    is_open_ = true;
    
    // Create namespace if doesn't exist
    if (data_.find(current_namespace_) == data_.end()) {
      data_[current_namespace_] = std::map<String, float>();
      string_data_[current_namespace_] = std::map<String, String>();
    }
    return true;
  }

  void end() override {
    is_open_ = false;
    current_namespace_ = "";
  }

  bool isKey(const char* key) override {
    if (!is_open_) return false;
    return data_[current_namespace_].find(key) != data_[current_namespace_].end() ||
           string_data_[current_namespace_].find(key) != string_data_[current_namespace_].end();
  }

  float getFloat(const char* key, float default_value = 0.0f) override {
    if (!is_open_) return default_value;
    auto& ns = data_[current_namespace_];
    if (ns.find(key) != ns.end()) {
      return ns[key];
    }
    return default_value;
  }

  size_t putFloat(const char* key, float value) override {
    if (!is_open_ || readonly_) return 0;
    data_[current_namespace_][key] = value;
    return sizeof(float);
  }

  String getString(const char* key, const String& default_value = "") override {
    if (!is_open_) return default_value;
    auto& ns = string_data_[current_namespace_];
    if (ns.find(key) != ns.end()) {
      return ns[key];
    }
    return default_value;
  }

  size_t putString(const char* key, const String& value) override {
    if (!is_open_ || readonly_) return 0;
    string_data_[current_namespace_][key] = value;
    return value.length();
  }

  bool remove(const char* key) override {
    if (!is_open_ || readonly_) return false;
    bool removed = false;
    removed |= data_[current_namespace_].erase(key) > 0;
    removed |= string_data_[current_namespace_].erase(key) > 0;
    return removed;
  }

  bool clear() override {
    if (!is_open_ || readonly_) return false;
    data_[current_namespace_].clear();
    string_data_[current_namespace_].clear();
    return true;
  }

  // Test helpers
  bool isOpen() const { return is_open_; }
  String getCurrentNamespace() const { return current_namespace_; }
  size_t getKeyCount() const {
    if (!is_open_) return 0;
    auto data_it = data_.find(current_namespace_);
    auto string_it = string_data_.find(current_namespace_);
    size_t count = 0;
    if (data_it != data_.end()) count += data_it->second.size();
    if (string_it != string_data_.end()) count += string_it->second.size();
    return count;
  }

 private:
  bool is_open_;
  bool readonly_;
  String current_namespace_;
  std::map<String, std::map<String, float>> data_;
  std::map<String, std::map<String, String>> string_data_;
};

void setUp(void) {}
void tearDown(void) {}

void test_storage_interface_begin() {
    MockStorageProvider storage;
    TEST_ASSERT_FALSE(storage.isOpen());
    TEST_ASSERT_TRUE(storage.begin("test_namespace"));
    TEST_ASSERT_TRUE(storage.isOpen());
    TEST_ASSERT_EQUAL_STRING("test_namespace", storage.getCurrentNamespace().c_str());
}

void test_storage_interface_end() {
    MockStorageProvider storage;
    storage.begin("test");
    TEST_ASSERT_TRUE(storage.isOpen());
    storage.end();
    TEST_ASSERT_FALSE(storage.isOpen());
}

void test_storage_interface_begin_twice_fails() {
    MockStorageProvider storage;
    TEST_ASSERT_TRUE(storage.begin("test"));
    TEST_ASSERT_FALSE(storage.begin("test2"));  // Already open
    storage.end();
}

void test_storage_interface_put_get_float() {
    MockStorageProvider storage;
    storage.begin("test");
    
    TEST_ASSERT_EQUAL(sizeof(float), storage.putFloat("voltage", 12.5f));
    TEST_ASSERT_EQUAL_FLOAT(12.5f, storage.getFloat("voltage"));
    
    storage.end();
}

void test_storage_interface_get_float_default() {
    MockStorageProvider storage;
    storage.begin("test");
    
    // Key doesn't exist, should return default
    TEST_ASSERT_EQUAL_FLOAT(99.9f, storage.getFloat("nonexistent", 99.9f));
    
    storage.end();
}

void test_storage_interface_put_get_string() {
    MockStorageProvider storage;
    storage.begin("test");
    
    String value = "test_value";
    TEST_ASSERT_EQUAL(value.length(), storage.putString("name", value));
    TEST_ASSERT_EQUAL_STRING("test_value", storage.getString("name").c_str());
    
    storage.end();
}

void test_storage_interface_get_string_default() {
    MockStorageProvider storage;
    storage.begin("test");
    
    TEST_ASSERT_EQUAL_STRING("default", storage.getString("nonexistent", "default").c_str());
    
    storage.end();
}

void test_storage_interface_is_key() {
    MockStorageProvider storage;
    storage.begin("test");
    
    TEST_ASSERT_FALSE(storage.isKey("voltage"));
    storage.putFloat("voltage", 12.0f);
    TEST_ASSERT_TRUE(storage.isKey("voltage"));
    
    storage.end();
}

void test_storage_interface_remove() {
    MockStorageProvider storage;
    storage.begin("test");
    
    storage.putFloat("voltage", 12.0f);
    TEST_ASSERT_TRUE(storage.isKey("voltage"));
    TEST_ASSERT_TRUE(storage.remove("voltage"));
    TEST_ASSERT_FALSE(storage.isKey("voltage"));
    
    storage.end();
}

void test_storage_interface_clear() {
    MockStorageProvider storage;
    storage.begin("test");
    
    storage.putFloat("voltage", 12.0f);
    storage.putFloat("current", 5.0f);
    storage.putString("name", "battery");
    TEST_ASSERT_EQUAL(3, storage.getKeyCount());
    
    TEST_ASSERT_TRUE(storage.clear());
    TEST_ASSERT_EQUAL(0, storage.getKeyCount());
    
    storage.end();
}

void test_storage_interface_readonly_mode() {
    MockStorageProvider storage;
    
    // First write some data
    storage.begin("test", false);
    storage.putFloat("voltage", 12.0f);
    storage.end();
    
    // Open readonly
    storage.begin("test", true);
    TEST_ASSERT_EQUAL_FLOAT(12.0f, storage.getFloat("voltage"));
    
    // Write should fail in readonly mode
    TEST_ASSERT_EQUAL(0, storage.putFloat("current", 5.0f));
    TEST_ASSERT_FALSE(storage.remove("voltage"));
    TEST_ASSERT_FALSE(storage.clear());
    
    storage.end();
}

void test_storage_interface_multiple_namespaces() {
    MockStorageProvider storage;
    
    // Write to namespace1
    storage.begin("namespace1");
    storage.putFloat("voltage", 12.0f);
    storage.end();
    
    // Write to namespace2
    storage.begin("namespace2");
    storage.putFloat("voltage", 24.0f);
    storage.end();
    
    // Verify namespace1 still has correct value
    storage.begin("namespace1");
    TEST_ASSERT_EQUAL_FLOAT(12.0f, storage.getFloat("voltage"));
    storage.end();
    
    // Verify namespace2 has its value
    storage.begin("namespace2");
    TEST_ASSERT_EQUAL_FLOAT(24.0f, storage.getFloat("voltage"));
    storage.end();
}

void test_storage_interface_overwrite_value() {
    MockStorageProvider storage;
    storage.begin("test");
    
    storage.putFloat("voltage", 12.0f);
    TEST_ASSERT_EQUAL_FLOAT(12.0f, storage.getFloat("voltage"));
    
    storage.putFloat("voltage", 13.5f);
    TEST_ASSERT_EQUAL_FLOAT(13.5f, storage.getFloat("voltage"));
    
    storage.end();
}

void test_storage_interface_battery_config_keys() {
    MockStorageProvider storage;
    storage.begin("battcfg");
    
    // Simulate battery configuration storage
    storage.putFloat("house_ah", 180.5f);
    storage.putFloat("house_marked", 200.0f);
    storage.putFloat("house_current", 195.0f);
    storage.putFloat("house_charge", 98.0f);
    storage.putFloat("house_discharge", 99.5f);
    
    TEST_ASSERT_EQUAL_FLOAT(180.5f, storage.getFloat("house_ah"));
    TEST_ASSERT_EQUAL_FLOAT(200.0f, storage.getFloat("house_marked"));
    TEST_ASSERT_EQUAL_FLOAT(195.0f, storage.getFloat("house_current"));
    TEST_ASSERT_EQUAL_FLOAT(98.0f, storage.getFloat("house_charge"));
    TEST_ASSERT_EQUAL_FLOAT(99.5f, storage.getFloat("house_discharge"));
    
    storage.end();
}

void test_storage_interface_operations_without_begin() {
    MockStorageProvider storage;
    
    // All operations should fail gracefully when not open
    TEST_ASSERT_FALSE(storage.isKey("test"));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, storage.getFloat("test"));
    TEST_ASSERT_EQUAL(0, storage.putFloat("test", 1.0f));
    TEST_ASSERT_EQUAL_STRING("", storage.getString("test").c_str());
    TEST_ASSERT_EQUAL(0, storage.putString("test", "value"));
    TEST_ASSERT_FALSE(storage.remove("test"));
    TEST_ASSERT_FALSE(storage.clear());
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_storage_interface_begin);
    RUN_TEST(test_storage_interface_end);
    RUN_TEST(test_storage_interface_begin_twice_fails);
    RUN_TEST(test_storage_interface_put_get_float);
    RUN_TEST(test_storage_interface_get_float_default);
    RUN_TEST(test_storage_interface_put_get_string);
    RUN_TEST(test_storage_interface_get_string_default);
    RUN_TEST(test_storage_interface_is_key);
    RUN_TEST(test_storage_interface_remove);
    RUN_TEST(test_storage_interface_clear);
    RUN_TEST(test_storage_interface_readonly_mode);
    RUN_TEST(test_storage_interface_multiple_namespaces);
    RUN_TEST(test_storage_interface_overwrite_value);
    RUN_TEST(test_storage_interface_battery_config_keys);
    RUN_TEST(test_storage_interface_operations_without_begin);
    
    UNITY_END();
}

void loop() {}
