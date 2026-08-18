#pragma once

#include "driver/i2c_master.h"

namespace oilgauge {

[[nodiscard]] bool startAds1115Diagnostics(i2c_master_bus_handle_t bus);

}  // namespace oilgauge
