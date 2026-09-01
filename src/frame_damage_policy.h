#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace oilgauge {

inline constexpr int kDamageFrameWidth = 480;
inline constexpr int kDamageFrameHeight = 480;
inline constexpr int kDamageTileSize = 32;
inline constexpr int kDamageTileColumns =
    (kDamageFrameWidth + kDamageTileSize - 1) / kDamageTileSize;
inline constexpr int kDamageTileRows =
    (kDamageFrameHeight + kDamageTileSize - 1) / kDamageTileSize;
inline constexpr std::uint32_t kDamageTileCount =
    kDamageTileColumns * kDamageTileRows;
inline constexpr std::size_t kDamageWordCount =
    (kDamageTileCount + 63U) / 64U;

struct DamageArea {
  int x1;
  int y1;
  int x2;
  int y2;
};

class DamageTiles {
 public:
  constexpr void clear() { words_ = {}; }

  [[nodiscard]] constexpr bool empty() const {
    for (const std::uint64_t word : words_) {
      if (word != 0) {
        return false;
      }
    }
    return true;
  }

  constexpr void markArea(DamageArea area) {
    if (area.x2 < 0 || area.y2 < 0 || area.x1 >= kDamageFrameWidth ||
        area.y1 >= kDamageFrameHeight || area.x1 > area.x2 ||
        area.y1 > area.y2) {
      return;
    }
    area.x1 = area.x1 < 0 ? 0 : area.x1;
    area.y1 = area.y1 < 0 ? 0 : area.y1;
    area.x2 = area.x2 >= kDamageFrameWidth ? kDamageFrameWidth - 1 : area.x2;
    area.y2 = area.y2 >= kDamageFrameHeight ? kDamageFrameHeight - 1 : area.y2;
    const int firstColumn = area.x1 / kDamageTileSize;
    const int lastColumn = area.x2 / kDamageTileSize;
    const int firstRow = area.y1 / kDamageTileSize;
    const int lastRow = area.y2 / kDamageTileSize;
    for (int row = firstRow; row <= lastRow; ++row) {
      for (int column = firstColumn; column <= lastColumn; ++column) {
        set(row * kDamageTileColumns + column);
      }
    }
  }

  constexpr void markFull() {
    clear();
    for (std::uint32_t tile = 0; tile < kDamageTileCount; ++tile) {
      set(static_cast<int>(tile));
    }
  }

  constexpr void merge(const DamageTiles& other) {
    for (std::size_t index = 0; index < words_.size(); ++index) {
      words_[index] |= other.words_[index];
    }
  }

  [[nodiscard]] constexpr bool marked(int row, int column) const {
    if (row < 0 || row >= kDamageTileRows || column < 0 ||
        column >= kDamageTileColumns) {
      return false;
    }
    const int tile = row * kDamageTileColumns + column;
    return (words_[static_cast<std::size_t>(tile) / 64U] &
            (std::uint64_t{1} << (static_cast<unsigned>(tile) % 64U))) != 0;
  }

  [[nodiscard]] constexpr std::uint32_t tileCount() const {
    std::uint32_t count = 0;
    for (std::uint64_t word : words_) {
      while (word != 0) {
        count += static_cast<std::uint32_t>(word & 1U);
        word >>= 1U;
      }
    }
    return count;
  }

 private:
  constexpr void set(int tile) {
    words_[static_cast<std::size_t>(tile) / 64U] |=
        std::uint64_t{1} << (static_cast<unsigned>(tile) % 64U);
  }

  std::array<std::uint64_t, kDamageWordCount> words_{};
};

struct DamageHistoryEntry {
  std::uint64_t generation = 0;
  DamageTiles tiles{};
};

template <std::size_t EntryCount>
[[nodiscard]] constexpr DamageTiles damageSince(
    const DamageHistoryEntry (&history)[EntryCount],
    std::size_t validEntries,
    std::uint64_t frameGeneration,
    std::uint64_t targetGeneration,
    bool& complete) {
  DamageTiles result;
  complete = true;
  if (targetGeneration <= frameGeneration) {
    return result;
  }
  const std::size_t boundedEntries =
      validEntries < EntryCount ? validEntries : EntryCount;
  for (std::uint64_t generation = frameGeneration + 1;
       generation <= targetGeneration;
       ++generation) {
    const DamageHistoryEntry* found = nullptr;
    for (std::size_t index = 0; index < boundedEntries; ++index) {
      if (history[index].generation == generation) {
        found = &history[index];
        break;
      }
    }
    if (found == nullptr) {
      complete = false;
      result.markFull();
      return result;
    }
    result.merge(found->tiles);
  }
  return result;
}

}  // namespace oilgauge
