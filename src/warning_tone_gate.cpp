#include "warning_tone_gate.h"

namespace oilgauge {

WarningToneCommand WarningToneGate::update(bool warningActive) {
  if (warningActive == warningActive_) {
    return WarningToneCommand::none;
  }
  warningActive_ = warningActive;
  return warningActive ? WarningToneCommand::startLoop
                       : WarningToneCommand::stopLoop;
}

bool WarningToneGate::warningActive() const {
  return warningActive_;
}

}  // namespace oilgauge
