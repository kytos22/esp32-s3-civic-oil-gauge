#pragma once

namespace oilgauge {

[[nodiscard]] bool initWarningAudio();
[[nodiscard]] bool warningAudioAvailable();
void setWarningAudioEnabled(bool enabled);
[[nodiscard]] bool setWarningAudioVolume(int percent);
void setWarningAudioActive(bool active);
void requestWarningTone();

}  // namespace oilgauge
