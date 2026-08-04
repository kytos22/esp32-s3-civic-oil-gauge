#pragma once

#include <cstdint>

namespace oilgauge::board {

inline constexpr std::int8_t kLcdSdio0 = 4;
inline constexpr std::int8_t kLcdSdio1 = 5;
inline constexpr std::int8_t kLcdSdio2 = 6;
inline constexpr std::int8_t kLcdSdio3 = 7;
inline constexpr std::int8_t kLcdClock = 38;
inline constexpr std::int8_t kLcdReset = 39;
inline constexpr std::int8_t kLcdChipSelect = 12;
inline constexpr std::int16_t kLcdWidth = 480;
inline constexpr std::int16_t kLcdHeight = 480;

inline constexpr std::int8_t kI2cSda = 15;
inline constexpr std::int8_t kI2cScl = 14;
inline constexpr std::int8_t kTouchInterrupt = 11;
inline constexpr std::int8_t kTouchReset = 40;

inline constexpr std::int8_t kUartTx = 43;
inline constexpr std::int8_t kUartRx = 44;

inline constexpr std::uint8_t kAds1115Address = 0x48;

}  // namespace oilgauge::board
