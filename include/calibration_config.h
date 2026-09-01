#pragma once

#include "gauge_core.h"

namespace oilgauge::calibration {

// La presión sigue bloqueada hasta completar su caracterización eléctrica.
inline constexpr LinearCalibration kPressure{0.0, 0.0, false};

// Calibración NTC provisional de banco (2026-08-25), derivada únicamente de
// las medidas de este sensor/MTX-D: 2552 ohm a 27 C y beta ~= 3800 K,
// contrastada en el extremo alto con 89 ohm ~= 135 C, 86 ohm ~= 136 C y
// 84 ohm ~= 138 C. Es válida para ensayos con resistencias en A1, no para
// retirar todavía el reloj de referencia del vehículo.
inline constexpr SteinhartHartCalibration kTemperature{
    0.0012672904878823104,
    0.0002631578947368421,
    0.0,
    true};

inline constexpr FrontendConfig kFrontend{};

}  // namespace oilgauge::calibration
