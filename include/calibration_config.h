#pragma once

#include "gauge_core.h"

namespace oilgauge::calibration {

// Intencionadamente inválidas hasta medir el MTX-D y sus sensores.
// No sustituir por valores encontrados en sensores visualmente similares.
inline constexpr LinearCalibration kPressure{0.0, 0.0, false};

inline constexpr SteinhartHartCalibration kTemperature{
    0.0, 0.0, 0.0, false};

inline constexpr FrontendConfig kFrontend{};

}  // namespace oilgauge::calibration
