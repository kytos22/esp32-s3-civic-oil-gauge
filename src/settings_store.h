#pragma once

#include "gauge_settings.h"

namespace oilgauge {

[[nodiscard]] bool initSettingsStore();
[[nodiscard]] GaugeSettings loadGaugeSettings(
    const GaugeSettings& defaults);
[[nodiscard]] bool saveGaugeSettings(const GaugeSettings& settings);

}  // namespace oilgauge
