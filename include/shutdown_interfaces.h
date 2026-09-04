#pragma once

#include <cstdint>

namespace sensesp {

/**
 * @brief Abstraction over the current time, in milliseconds
 *
 * Allows ShutdownCoordinator to be tested without a real clock source.
 */
class IClock {
 public:
  virtual ~IClock() = default;
  virtual uint64_t now_ms() const = 0;
};

/**
 * @brief Abstraction over entering deep sleep
 *
 * Allows ShutdownCoordinator to be tested without halting the process.
 */
class ISleepController {
 public:
  virtual ~ISleepController() = default;
  virtual void enter_deep_sleep() = 0;
};

}  // namespace sensesp
