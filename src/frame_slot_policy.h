#pragma once

#include <cstddef>
#include <cstdint>

namespace oilgauge {

enum class FrameSlotState : std::uint8_t {
  free,
  rendering,
  ready,
  inFlight,
};

struct FrameSlotMetadata {
  FrameSlotState state = FrameSlotState::free;
  std::uint64_t generation = 0;
};

// Keep exactly one coherent READY frame. A new render starts only after TE has
// handed that frame to DMA, leaving a FREE canvas for the next generation.
template <std::size_t SlotCount>
[[nodiscard]] constexpr int selectRenderSlot(
    const FrameSlotMetadata (&slots)[SlotCount]) {
  for (const FrameSlotMetadata& slot : slots) {
    if (slot.state == FrameSlotState::ready ||
        slot.state == FrameSlotState::rendering) {
      return -1;
    }
  }

  int newestFree = -1;
  for (std::size_t index = 0; index < SlotCount; ++index) {
    if (slots[index].state != FrameSlotState::free) {
      continue;
    }
    if (newestFree < 0 ||
        slots[index].generation >
            slots[static_cast<std::size_t>(newestFree)].generation) {
      newestFree = static_cast<int>(index);
    }
  }
  return newestFree;
}

// Presentation consumes the coherent READY generation. The selector remains
// defensive if corrupted/test state ever contains more than one READY slot.
template <std::size_t SlotCount>
[[nodiscard]] constexpr int selectNewestReadySlot(
    const FrameSlotMetadata (&slots)[SlotCount]) {
  int newestReady = -1;
  for (std::size_t index = 0; index < SlotCount; ++index) {
    if (slots[index].state != FrameSlotState::ready) {
      continue;
    }
    if (newestReady < 0 ||
        slots[index].generation >
            slots[static_cast<std::size_t>(newestReady)].generation) {
      newestReady = static_cast<int>(index);
    }
  }
  return newestReady;
}

}  // namespace oilgauge
