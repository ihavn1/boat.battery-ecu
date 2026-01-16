#include "sensor_factory.h"

namespace sensesp {

// Define static INA226 hardware instances
INA226 SensorFactory::house_battery_ina_(0x40);
INA226 SensorFactory::starter_battery_ina_(0x41);

}  // namespace sensesp
