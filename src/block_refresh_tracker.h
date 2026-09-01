#pragma once

namespace oilgauge {

enum class BlockRefreshOutcome {
  ready,
  retry,
};

class BlockRefreshTracker {
 public:
  void begin() {
    sawFlush_ = false;
    sawLastFlush_ = false;
  }

  void onFlush(bool isLast) {
    sawFlush_ = true;
    sawLastFlush_ = sawLastFlush_ || isLast;
  }

  [[nodiscard]] bool sawFlush() const { return sawFlush_; }
  [[nodiscard]] bool sawLastFlush() const { return sawLastFlush_; }

  [[nodiscard]] BlockRefreshOutcome finish() const {
    return sawFlush_ && sawLastFlush_ ? BlockRefreshOutcome::ready
                                      : BlockRefreshOutcome::retry;
  }

 private:
  bool sawFlush_ = false;
  bool sawLastFlush_ = false;
};

}  // namespace oilgauge
