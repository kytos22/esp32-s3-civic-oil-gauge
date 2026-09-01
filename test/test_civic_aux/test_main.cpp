#include <unity.h>

#include <array>
#include <cstddef>
#include <cstdint>

#include "automatic_brightness.h"
#include "civic_aux_receiver.h"
#include "gauge_settings.h"

namespace {

using namespace oilgauge;

constexpr std::uint8_t kFullAutomaticMinimumPercent = 5;
constexpr std::uint8_t kFullAutomaticMaximumPercent = 100;

// Byte-exact copies from ESP32 Civic Auxiliary Hub at
// test_vectors/civic_aux_v1_vectors.json, SHA-256
// 37A25643C9247650FDF557E4099A7F627F4D148FEB49B4CE274D19FE358E16FF.
constexpr std::array<std::uint8_t, 28> kHubStatusVector{{
    0xA5, 0x5A, 0x01, 0x01, 0x01, 0x07, 0x0C, 0x00,
    0x34, 0x12, 0x04, 0x03, 0x02, 0x01, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
    0x00, 0x00, 0xA6, 0x44,
}};

constexpr std::array<std::uint8_t, 28> kAmbientLightVector{{
    0xA5, 0x5A, 0x01, 0x10, 0x10, 0x07, 0x0C, 0x01,
    0xAB, 0x00, 0xE8, 0x03, 0x00, 0x00, 0x01, 0x01,
    0x34, 0x12, 0x78, 0x56, 0x19, 0x00, 0x40, 0xE2,
    0x01, 0x00, 0x96, 0x50,
}};

struct EncodedFrame {
  std::array<std::uint8_t, kCivicAuxMaximumFrameSize> bytes{};
  std::size_t length = 0;
};

void writeU16Le(std::uint8_t* output, std::uint16_t value) {
  output[0] = static_cast<std::uint8_t>(value & 0xFFU);
  output[1] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}

void writeU32Le(std::uint8_t* output, std::uint32_t value) {
  output[0] = static_cast<std::uint8_t>(value & 0xFFU);
  output[1] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
  output[2] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
  output[3] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

EncodedFrame makeFrame(std::uint8_t type,
                       std::uint8_t source,
                       std::uint8_t targetMask,
                       std::uint8_t flags,
                       std::uint16_t sequence,
                       std::uint32_t uptimeMs,
                       const std::array<std::uint8_t, 48>& payload,
                       std::uint8_t payloadLength,
                       std::uint8_t version = kCivicAuxProtocolVersion) {
  EncodedFrame frame{};
  frame.length = kCivicAuxMinimumFrameSize + payloadLength;
  frame.bytes[0] = kCivicAuxSync0;
  frame.bytes[1] = kCivicAuxSync1;
  frame.bytes[2] = version;
  frame.bytes[3] = type;
  frame.bytes[4] = source;
  frame.bytes[5] = targetMask;
  frame.bytes[6] = payloadLength;
  frame.bytes[7] = flags;
  writeU16Le(frame.bytes.data() + 8, sequence);
  writeU32Le(frame.bytes.data() + 10, uptimeMs);
  for (std::size_t index = 0; index < payloadLength; ++index) {
    frame.bytes[14 + index] = payload[index];
  }
  const std::uint16_t crc = civicAuxCrc16CcittFalse(
      frame.bytes.data() + 2, frame.length - 4U);
  writeU16Le(frame.bytes.data() + frame.length - 2U, crc);
  return frame;
}

EncodedFrame makeAmbient(std::uint16_t sequence,
                         std::uint32_t uptimeMs,
                         CivicAuxSensorState state,
                         std::uint8_t flags,
                         std::uint16_t ageMs,
                         std::uint32_t millilux,
                         std::uint8_t targetMask = 0x07,
                         std::uint8_t source = kCivicAuxAmbientLightSource,
                         std::uint8_t payloadLength = 12) {
  std::array<std::uint8_t, 48> payload{};
  payload[0] = static_cast<std::uint8_t>(state);
  payload[1] = static_cast<std::uint8_t>(CivicAuxRangeProfile::normal);
  writeU16Le(payload.data() + 2, 0x1234);
  writeU16Le(payload.data() + 4, 0x5678);
  writeU16Le(payload.data() + 6, ageMs);
  writeU32Le(payload.data() + 8, millilux);
  return makeFrame(kCivicAuxAmbientLightType,
                   source,
                   targetMask,
                   flags,
                   sequence,
                   uptimeMs,
                   payload,
                   payloadLength);
}

EncodedFrame makeHubStatus(std::uint16_t sequence,
                           std::uint32_t uptimeMs) {
  std::array<std::uint8_t, 48> payload{};
  writeU32Le(payload.data(), 1U);
  writeU32Le(payload.data() + 4, 0U);
  writeU32Le(payload.data() + 8, 2U);
  return makeFrame(kCivicAuxHubStatusType,
                   kCivicAuxHubSource,
                   0x07,
                   0,
                   sequence,
                   uptimeMs,
                   payload,
                   12);
}

template <std::size_t Size>
std::size_t feedParser(CivicAuxFrameParser& parser,
                       const std::array<std::uint8_t, Size>& bytes,
                       CivicAuxFrameV1& frame) {
  std::size_t completed = 0;
  for (const std::uint8_t byte : bytes) {
    if (parser.feed(byte, frame)) {
      ++completed;
    }
  }
  return completed;
}

void feedReceiver(CivicAuxReceiver& receiver,
                  const EncodedFrame& frame,
                  std::uint64_t localNowMs) {
  for (std::size_t index = 0; index < frame.length; ++index) {
    (void)receiver.feed(frame.bytes[index], localNowMs);
  }
}

void feedReceiver(CivicAuxReceiver& receiver,
                  const std::array<std::uint8_t, 28>& frame,
                  std::uint64_t localNowMs) {
  for (const std::uint8_t byte : frame) {
    (void)receiver.feed(byte, localNowMs);
  }
}

CivicAuxSnapshot usableSnapshot(std::uint32_t generation,
                                std::uint8_t consecutive,
                                std::uint32_t millilux,
                                std::uint64_t receivedAtMs,
                                std::uint64_t validUntilMs) {
  CivicAuxSnapshot snapshot{};
  snapshot.receiverRunning = true;
  snapshot.hasAmbientFrame = true;
  snapshot.latestAmbientUsable = true;
  snapshot.hasUsableLux = true;
  snapshot.consecutiveUsableAmbientFrames = consecutive;
  snapshot.filteredMillilux = millilux;
  snapshot.lastUsableMillilux = millilux;
  snapshot.ambientReceivedAtMs = receivedAtMs;
  snapshot.lastUsableReceivedAtMs = receivedAtMs;
  snapshot.lastUsableUntilMs = validUntilMs;
  snapshot.usableGeneration = generation;
  return snapshot;
}

void test_crc_standard_check_vector() {
  constexpr std::array<std::uint8_t, 9> check{{
      '1', '2', '3', '4', '5', '6', '7', '8', '9',
  }};
  TEST_ASSERT_EQUAL_HEX16(
      0x29B1, civicAuxCrc16CcittFalse(check.data(), check.size()));
}

void test_hub_status_vector_is_byte_exact_and_little_endian() {
  CivicAuxFrameParser parser;
  CivicAuxFrameV1 frame{};
  TEST_ASSERT_EQUAL_UINT32(1, feedParser(parser, kHubStatusVector, frame));
  TEST_ASSERT_EQUAL_HEX8(0x01, frame.type);
  TEST_ASSERT_EQUAL_HEX8(0x01, frame.source);
  TEST_ASSERT_EQUAL_HEX8(0x07, frame.targetMask);
  TEST_ASSERT_EQUAL_UINT16(0x1234, frame.sequence);
  TEST_ASSERT_EQUAL_UINT32(0x01020304, frame.uptimeMs);
  TEST_ASSERT_EQUAL_HEX8(0x01, frame.payload[0]);
  TEST_ASSERT_EQUAL_HEX8(0x02, frame.payload[8]);
  TEST_ASSERT_EQUAL_UINT32(1, parser.diagnostics().structurallyValidFrames);
}

void test_ambient_vector_is_byte_exact_and_little_endian() {
  CivicAuxFrameParser parser;
  CivicAuxFrameV1 frame{};
  TEST_ASSERT_EQUAL_UINT32(1, feedParser(parser, kAmbientLightVector, frame));
  CivicAuxAmbientLightPayload payload{};
  TEST_ASSERT_TRUE(decodeCivicAuxAmbientLight(frame, payload));
  TEST_ASSERT_EQUAL_UINT8(1, payload.sensorState);
  TEST_ASSERT_EQUAL_UINT8(1, payload.rangeProfile);
  TEST_ASSERT_EQUAL_HEX16(0x1234, payload.alsRaw);
  TEST_ASSERT_EQUAL_HEX16(0x5678, payload.whiteRaw);
  TEST_ASSERT_EQUAL_UINT16(25, payload.sampleAgeMs);
  TEST_ASSERT_EQUAL_UINT32(123456, payload.filteredMillilux);
}

void test_parser_resynchronizes_across_noise_between_vectors() {
  CivicAuxFrameParser parser;
  CivicAuxFrameV1 frame{};
  constexpr std::array<std::uint8_t, 6> noise{{0x00, 0xFF, 0xA5,
                                               0x11, 0x5A, 0x42}};
  std::size_t completed = feedParser(parser, noise, frame);
  completed += feedParser(parser, kHubStatusVector, frame);
  completed += feedParser(parser, noise, frame);
  completed += feedParser(parser, kAmbientLightVector, frame);
  TEST_ASSERT_EQUAL_UINT32(2, completed);
  TEST_ASSERT_GREATER_THAN_UINT32(0, parser.diagnostics().discardedBytes);
}

void test_truncated_frame_followed_by_valid_frame_resynchronizes() {
  CivicAuxFrameParser parser;
  CivicAuxFrameV1 frame{};
  for (std::size_t index = 0; index < 20; ++index) {
    TEST_ASSERT_FALSE(parser.feed(kAmbientLightVector[index], frame));
  }
  TEST_ASSERT_EQUAL_UINT32(1, feedParser(parser, kAmbientLightVector, frame));
  TEST_ASSERT_EQUAL_HEX8(kCivicAuxAmbientLightType, frame.type);
  TEST_ASSERT_GREATER_THAN_UINT32(0, parser.diagnostics().invalidEvents);
}

void test_bad_crc_is_rejected_before_next_valid_frame() {
  CivicAuxFrameParser parser;
  CivicAuxFrameV1 frame{};
  auto corrupt = kAmbientLightVector;
  corrupt[27] ^= 0x80U;
  TEST_ASSERT_EQUAL_UINT32(0, feedParser(parser, corrupt, frame));
  TEST_ASSERT_EQUAL_UINT32(1, feedParser(parser, kAmbientLightVector, frame));
  TEST_ASSERT_GREATER_THAN_UINT32(0, parser.diagnostics().crcErrors);
}

void test_incompatible_version_is_rejected() {
  std::array<std::uint8_t, 48> payload{};
  const EncodedFrame incompatible = makeFrame(
      0x77, 0x55, kCivicAuxTargetOil, 0, 1, 10, payload, 0, 2);
  CivicAuxFrameParser parser;
  CivicAuxFrameV1 frame{};
  for (std::size_t index = 0; index < incompatible.length; ++index) {
    TEST_ASSERT_FALSE(parser.feed(incompatible.bytes[index], frame));
  }
  TEST_ASSERT_EQUAL_UINT32(1, parser.diagnostics().versionErrors);
}

void test_payload_over_48_is_rejected_and_parser_recovers() {
  constexpr std::array<std::uint8_t, 7> oversizedHeader{{
      0xA5, 0x5A, 0x01, 0x10, 0x10, 0x01, 49,
  }};
  CivicAuxFrameParser parser;
  CivicAuxFrameV1 frame{};
  TEST_ASSERT_EQUAL_UINT32(0, feedParser(parser, oversizedHeader, frame));
  TEST_ASSERT_EQUAL_UINT32(1, feedParser(parser, kAmbientLightVector, frame));
  TEST_ASSERT_GREATER_THAN_UINT32(0, parser.diagnostics().lengthErrors);
}

void test_wrong_ambient_length_and_wrong_target_are_rejected() {
  CivicAuxReceiver receiver;
  const EncodedFrame wrongLength = makeAmbient(
      1, 100, CivicAuxSensorState::valid, kCivicAuxFlagDataValid, 0, 1000,
      0x07, kCivicAuxAmbientLightSource, 11);
  const EncodedFrame wrongTarget = makeAmbient(
      2, 200, CivicAuxSensorState::valid, kCivicAuxFlagDataValid, 0, 1000,
      0x02);
  feedReceiver(receiver, wrongLength, 100);
  feedReceiver(receiver, wrongTarget, 200);
  TEST_ASSERT_FALSE(receiver.snapshot().hasAmbientFrame);
  TEST_ASSERT_EQUAL_UINT32(
      1, receiver.snapshot().diagnostics.knownLengthErrors);
  TEST_ASSERT_EQUAL_UINT32(1, receiver.snapshot().diagnostics.targetMisses);
}

void test_unknown_valid_type_is_consumed_and_ignored() {
  std::array<std::uint8_t, 48> payload{};
  const EncodedFrame unknown = makeFrame(
      0x77, 0x55, kCivicAuxTargetOil, 0, 1, 100, payload, 3);
  CivicAuxReceiver receiver;
  feedReceiver(receiver, unknown, 100);
  TEST_ASSERT_EQUAL_UINT32(1, receiver.snapshot().diagnostics.unknownFrames);
  TEST_ASSERT_EQUAL_UINT32(1, receiver.snapshot().diagnostics.acceptedFrames);
  TEST_ASSERT_FALSE(receiver.snapshot().hasAmbientFrame);
}

void test_flags_state_and_communicated_freshness_gate_lux() {
  CivicAuxReceiver receiver;
  feedReceiver(receiver,
               makeAmbient(1, 100, CivicAuxSensorState::valid, 0, 0, 1000),
               100);
  feedReceiver(receiver,
               makeAmbient(2, 200, CivicAuxSensorState::initializing,
                           kCivicAuxFlagDataValid, 0, 1000),
               200);
  feedReceiver(receiver,
               makeAmbient(3, 300, CivicAuxSensorState::degraded,
                           kCivicAuxFlagDataValid, 1999, 2000),
               300);
  TEST_ASSERT_TRUE(receiver.snapshot().latestAmbientUsable);
  TEST_ASSERT_EQUAL_UINT8(1,
                          receiver.snapshot().consecutiveUsableAmbientFrames);
  feedReceiver(receiver,
               makeAmbient(4, 400, CivicAuxSensorState::valid,
                           kCivicAuxFlagDataValid, 2000, 3000),
               400);
  feedReceiver(receiver,
               makeAmbient(5, 500, CivicAuxSensorState::missing,
                           kCivicAuxFlagDataValid, 0, 4000),
               500);
  TEST_ASSERT_FALSE(receiver.snapshot().latestAmbientUsable);
  TEST_ASSERT_EQUAL_UINT8(0,
                          receiver.snapshot().consecutiveUsableAmbientFrames);
  TEST_ASSERT_EQUAL_UINT32(
      4, receiver.snapshot().diagnostics.unusableAmbientFrames);
  TEST_ASSERT_EQUAL_UINT32(1,
                           receiver.snapshot().diagnostics.usableAmbientFrames);
}

void test_hub_status_never_renews_lux_freshness() {
  CivicAuxReceiver receiver;
  feedReceiver(receiver,
               makeAmbient(1, 100, CivicAuxSensorState::valid,
                           kCivicAuxFlagDataValid, 100, 123000),
               1000);
  const std::uint64_t deadline = receiver.snapshot().lastUsableUntilMs;
  const std::uint32_t generation = receiver.snapshot().usableGeneration;
  feedReceiver(receiver, makeHubStatus(2, 200), 1500);
  TEST_ASSERT_EQUAL_UINT64(deadline, receiver.snapshot().lastUsableUntilMs);
  TEST_ASSERT_EQUAL_UINT32(generation,
                           receiver.snapshot().usableGeneration);
  TEST_ASSERT_EQUAL_UINT32(1,
                           receiver.snapshot().diagnostics.hubStatusFrames);
}

void test_sequence_wrap_is_continuous_but_hub_restart_resets_recovery() {
  CivicAuxReceiver wrapReceiver;
  feedReceiver(wrapReceiver,
               makeAmbient(0xFFFF, 1000, CivicAuxSensorState::valid,
                           kCivicAuxFlagDataValid, 0, 1000),
               100);
  feedReceiver(wrapReceiver,
               makeAmbient(0, 1200, CivicAuxSensorState::valid,
                           kCivicAuxFlagDataValid, 0, 2000),
               300);
  TEST_ASSERT_EQUAL_UINT32(1,
                           wrapReceiver.snapshot().diagnostics.sequenceWraps);
  TEST_ASSERT_EQUAL_UINT8(
      2, wrapReceiver.snapshot().consecutiveUsableAmbientFrames);

  CivicAuxReceiver restartReceiver;
  feedReceiver(restartReceiver,
               makeAmbient(10, 5000, CivicAuxSensorState::valid,
                           kCivicAuxFlagDataValid, 0, 1000),
               100);
  feedReceiver(restartReceiver,
               makeAmbient(0, 100, CivicAuxSensorState::valid,
                           kCivicAuxFlagDataValid, 0, 2000),
               300);
  TEST_ASSERT_EQUAL_UINT32(
      1, restartReceiver.snapshot().diagnostics.hubRestarts);
  TEST_ASSERT_EQUAL_UINT8(
      1, restartReceiver.snapshot().consecutiveUsableAmbientFrames);
}

void test_invalid_frame_breaks_two_sample_recovery_sequence() {
  CivicAuxReceiver receiver;
  feedReceiver(receiver,
               makeAmbient(1, 100, CivicAuxSensorState::valid,
                           kCivicAuxFlagDataValid, 0, 1000),
               100);
  auto corrupt = makeAmbient(2, 200, CivicAuxSensorState::valid,
                             kCivicAuxFlagDataValid, 0, 2000);
  corrupt.bytes[corrupt.length - 1U] ^= 0x01U;
  feedReceiver(receiver, corrupt, 200);
  feedReceiver(receiver,
               makeAmbient(3, 300, CivicAuxSensorState::valid,
                           kCivicAuxFlagDataValid, 0, 3000),
               300);
  TEST_ASSERT_EQUAL_UINT8(
      1, receiver.snapshot().consecutiveUsableAmbientFrames);
  TEST_ASSERT_GREATER_THAN_UINT32(
      0, receiver.snapshot().diagnostics.parser.crcErrors);
}

void test_continued_invalid_traffic_arms_one_second_fallback() {
  CivicAuxReceiver receiver;
  auto corrupt = makeAmbient(1, 100, CivicAuxSensorState::valid,
                             kCivicAuxFlagDataValid, 0, 1000);
  corrupt.bytes[corrupt.length - 1U] ^= 0x01U;
  feedReceiver(receiver, corrupt, 100);
  TEST_ASSERT_FALSE(receiver.snapshot().invalidTrafficFallback);
  feedReceiver(receiver, corrupt, 1100);
  TEST_ASSERT_TRUE(receiver.snapshot().invalidTrafficFallback);
}

void test_brightness_curve_exact_points_and_intermediate_values() {
  constexpr std::array<std::uint32_t, 9> lux{{
      0, 1, 5, 20, 100, 500, 2000, 10000, 30000,
  }};
  constexpr std::array<std::uint8_t, 9> expected{{
      5, 7, 12, 20, 35, 50, 65, 85, 100,
  }};
  for (std::size_t index = 0; index < lux.size(); ++index) {
    TEST_ASSERT_EQUAL_UINT8(
        expected[index],
        automaticBrightnessPercentForMillilux(lux[index] * 1000U));
  }
  TEST_ASSERT_EQUAL_UINT8(9, automaticBrightnessPercentForMillilux(2000));
  TEST_ASSERT_EQUAL_UINT8(16, automaticBrightnessPercentForMillilux(10000));
  TEST_ASSERT_EQUAL_UINT8(44, automaticBrightnessPercentForMillilux(250000));
  TEST_ASSERT_EQUAL_UINT8(94,
                          automaticBrightnessPercentForMillilux(20000000));
}

void test_brightness_curve_is_monotonic_and_saturates() {
  std::uint8_t previous = 0;
  for (std::uint32_t lux = 0; lux <= 30000; ++lux) {
    const std::uint8_t current =
        automaticBrightnessPercentForMillilux(lux * 1000U);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT8(previous, current);
    previous = current;
  }
  TEST_ASSERT_EQUAL_UINT8(
      100, automaticBrightnessPercentForMillilux(0xFFFFFFFFU));
}

void test_brightness_mode_defaults_migrates_and_round_trips() {
  const GaugeSettings defaults{};
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(BrightnessMode::automatic),
      static_cast<std::uint8_t>(defaults.brightnessMode));
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(BrightnessMode::automatic),
      static_cast<std::uint8_t>(brightnessModeFromStoredValue(false, 1)));
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(BrightnessMode::manual),
      static_cast<std::uint8_t>(brightnessModeFromStoredValue(true, 1)));
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(BrightnessMode::automatic),
      static_cast<std::uint8_t>(brightnessModeFromStoredValue(true, 99)));
  TEST_ASSERT_EQUAL_UINT8(
      1, brightnessModeStoredValue(BrightnessMode::manual));
  TEST_ASSERT_EQUAL_UINT8(20,
                          defaults.automaticBrightnessMinimumPercent);
  TEST_ASSERT_EQUAL_UINT8(100,
                          defaults.automaticBrightnessMaximumPercent);
  TEST_ASSERT_EQUAL_INT8(0, defaults.automaticBrightnessBiasPercent);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(UiLanguage::spanish),
      static_cast<std::uint8_t>(defaults.language));
}

void test_auto_starts_on_backup_and_requires_two_new_samples() {
  AutomaticBrightnessController controller;
  controller.reset(BrightnessMode::automatic,
                   55,
                   kFullAutomaticMinimumPercent,
                   kFullAutomaticMaximumPercent,
                   0);
  CivicAuxSnapshot empty{};
  auto status = controller.update(empty, 0);
  TEST_ASSERT_EQUAL_UINT8(55, status.appliedPercent);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(AutomaticBrightnessState::waitingForSamples),
      static_cast<std::uint8_t>(status.state));

  CivicAuxSnapshot first = usableSnapshot(1, 1, 2'000'000, 100, 2100);
  status = controller.update(first, 100);
  TEST_ASSERT_EQUAL_UINT8(55, status.appliedPercent);
  CivicAuxSnapshot second = usableSnapshot(2, 2, 2'000'000, 300, 2300);
  status = controller.update(second, 300);
  TEST_ASSERT_EQUAL_UINT8(65, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(63, status.appliedPercent);
  status = controller.update(second, 350);
  TEST_ASSERT_EQUAL_UINT8(65, status.appliedPercent);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(AutomaticBrightnessState::automatic),
      static_cast<std::uint8_t>(status.state));
}

void test_manual_mode_ignores_all_lux_and_uses_backup_slider() {
  AutomaticBrightnessController controller;
  controller.reset(BrightnessMode::manual, 40, 20, 30, 0);
  CivicAuxSnapshot bright = usableSnapshot(2, 2, 30'000'000, 100, 5000);
  auto status = controller.update(bright, 100);
  TEST_ASSERT_EQUAL_UINT8(40, status.appliedPercent);
  TEST_ASSERT_FALSE(status.automaticPercentAvailable);
  controller.setPreferences(BrightnessMode::manual, 72, 20, 30, 200);
  status = controller.update(bright, 200);
  TEST_ASSERT_EQUAL_UINT8(72, status.appliedPercent);
  TEST_ASSERT_FALSE(status.persistenceRequested);
}

void test_invalid_traffic_falls_back_smoothly_over_1500_ms() {
  AutomaticBrightnessController controller;
  controller.reset(BrightnessMode::automatic,
                   20,
                   kFullAutomaticMinimumPercent,
                   kFullAutomaticMaximumPercent,
                   0);
  CivicAuxSnapshot first = usableSnapshot(1, 1, 30'000'000, 100, 5000);
  (void)controller.update(first, 100);
  CivicAuxSnapshot active = usableSnapshot(2, 2, 30'000'000, 300, 5000);
  auto status = controller.update(active, 300);
  TEST_ASSERT_EQUAL_UINT8(100, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(28, status.appliedPercent);
  status = controller.update(active, 2300);
  TEST_ASSERT_EQUAL_UINT8(100, status.appliedPercent);

  active.invalidTrafficFallback = true;
  active.consecutiveUsableAmbientFrames = 0;
  status = controller.update(active, 3000);
  TEST_ASSERT_EQUAL_UINT8(100, status.appliedPercent);
  status = controller.update(active, 3750);
  TEST_ASSERT_EQUAL_UINT8(60, status.appliedPercent);
  status = controller.update(active, 4500);
  TEST_ASSERT_EQUAL_UINT8(20, status.appliedPercent);
}

void test_two_second_timeout_falls_back_and_two_samples_recover() {
  AutomaticBrightnessController controller;
  controller.reset(BrightnessMode::automatic,
                   35,
                   kFullAutomaticMinimumPercent,
                   kFullAutomaticMaximumPercent,
                   0);
  CivicAuxSnapshot first = usableSnapshot(1, 1, 10'000'000, 100, 2100);
  (void)controller.update(first, 100);
  CivicAuxSnapshot active = usableSnapshot(2, 2, 10'000'000, 200, 2200);
  auto status = controller.update(active, 200);
  TEST_ASSERT_EQUAL_UINT8(85, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(39, status.appliedPercent);
  status = controller.update(active, 1350);
  TEST_ASSERT_EQUAL_UINT8(85, status.appliedPercent);
  status = controller.update(active, 2200);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(AutomaticBrightnessState::fallback),
      static_cast<std::uint8_t>(status.state));
  status = controller.update(active, 3700);
  TEST_ASSERT_EQUAL_UINT8(35, status.appliedPercent);

  CivicAuxSnapshot recovering = usableSnapshot(3, 1, 500'000, 3800, 5800);
  status = controller.update(recovering, 3800);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(AutomaticBrightnessState::fallback),
      static_cast<std::uint8_t>(status.state));
  recovering = usableSnapshot(4, 2, 500'000, 4000, 6000);
  status = controller.update(recovering, 4000);
  TEST_ASSERT_EQUAL_UINT8(50, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(43, status.appliedPercent);
  status = controller.update(recovering, 4175);
  TEST_ASSERT_EQUAL_UINT8(50, status.appliedPercent);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(AutomaticBrightnessState::automatic),
      static_cast<std::uint8_t>(status.state));
}

void test_hub_restart_requires_two_post_restart_samples() {
  AutomaticBrightnessController controller;
  controller.reset(BrightnessMode::automatic,
                   55,
                   kFullAutomaticMinimumPercent,
                   kFullAutomaticMaximumPercent,
                   0);
  CivicAuxSnapshot first = usableSnapshot(1, 1, 2'000'000, 100, 5000);
  (void)controller.update(first, 100);
  CivicAuxSnapshot active = usableSnapshot(2, 2, 2'000'000, 300, 5000);
  (void)controller.update(active, 300);

  CivicAuxSnapshot restarted = usableSnapshot(3, 1, 500'000, 500, 2500);
  restarted.diagnostics.hubRestarts = 1;
  auto status = controller.update(restarted, 500);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(AutomaticBrightnessState::fallback),
      static_cast<std::uint8_t>(status.state));
  restarted = usableSnapshot(4, 2, 500'000, 700, 2700);
  restarted.diagnostics.hubRestarts = 1;
  status = controller.update(restarted, 700);
  TEST_ASSERT_EQUAL_UINT8(50, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(58, status.appliedPercent);
  status = controller.update(restarted, 1020);
  TEST_ASSERT_EQUAL_UINT8(50, status.appliedPercent);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(AutomaticBrightnessState::automatic),
      static_cast<std::uint8_t>(status.state));
}

void test_automatic_samples_never_request_settings_persistence() {
  AutomaticBrightnessController controller;
  controller.reset(BrightnessMode::automatic,
                   55,
                   kFullAutomaticMinimumPercent,
                   kFullAutomaticMaximumPercent,
                   0);
  for (std::uint32_t generation = 1; generation <= 20; ++generation) {
    const CivicAuxSnapshot snapshot = usableSnapshot(
        generation,
        static_cast<std::uint8_t>(generation >= 2 ? 2 : 1),
        generation * 100'000U,
        generation * 200U,
        generation * 200U + 2000U);
    const AutomaticBrightnessStatus status =
        controller.update(snapshot, generation * 200U);
    TEST_ASSERT_FALSE(status.persistenceRequested);
  }
}

void test_communicated_age_shortens_the_local_freshness_window() {
  CivicAuxReceiver receiver;
  AutomaticBrightnessController controller;
  controller.reset(BrightnessMode::automatic,
                   55,
                   kFullAutomaticMinimumPercent,
                   kFullAutomaticMaximumPercent,
                   0);

  feedReceiver(receiver,
               makeAmbient(1, 100, CivicAuxSensorState::valid,
                           kCivicAuxFlagDataValid, 1500, 10'000'000),
               1000);
  auto status = controller.update(receiver.snapshot(), 1000);
  TEST_ASSERT_EQUAL_UINT64(1500, receiver.snapshot().lastUsableUntilMs);
  TEST_ASSERT_EQUAL_UINT8(55, status.appliedPercent);

  feedReceiver(receiver,
               makeAmbient(2, 300, CivicAuxSensorState::valid,
                           kCivicAuxFlagDataValid, 1500, 10'000'000),
               1200);
  status = controller.update(receiver.snapshot(), 1200);
  TEST_ASSERT_EQUAL_UINT64(1700, receiver.snapshot().lastUsableUntilMs);
  TEST_ASSERT_EQUAL_UINT8(85, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(63, status.appliedPercent);
  TEST_ASSERT_TRUE(status.luxFresh);

  status = controller.update(receiver.snapshot(), 1700);
  TEST_ASSERT_FALSE(status.luxFresh);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(AutomaticBrightnessState::fallback),
      static_cast<std::uint8_t>(status.state));
}

void test_manual_to_auto_requires_two_samples_received_after_selection() {
  AutomaticBrightnessController controller;
  controller.reset(BrightnessMode::manual,
                   42,
                   kFullAutomaticMinimumPercent,
                   kFullAutomaticMaximumPercent,
                   0);
  CivicAuxSnapshot existing =
      usableSnapshot(10, 10, 30'000'000, 100, 5000);
  (void)controller.update(existing, 100);

  controller.setPreferences(BrightnessMode::automatic,
                            42,
                            kFullAutomaticMinimumPercent,
                            kFullAutomaticMaximumPercent,
                            200);
  auto status = controller.update(existing, 200);
  TEST_ASSERT_EQUAL_UINT8(42, status.appliedPercent);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(AutomaticBrightnessState::waitingForSamples),
      static_cast<std::uint8_t>(status.state));

  CivicAuxSnapshot first =
      usableSnapshot(11, 11, 30'000'000, 300, 2300);
  status = controller.update(first, 300);
  TEST_ASSERT_EQUAL_UINT8(42, status.appliedPercent);

  CivicAuxSnapshot second =
      usableSnapshot(12, 12, 30'000'000, 500, 2500);
  status = controller.update(second, 500);
  TEST_ASSERT_EQUAL_UINT8(100, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(50, status.appliedPercent);
  status = controller.update(second, 1750);
  TEST_ASSERT_EQUAL_UINT8(100, status.appliedPercent);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(AutomaticBrightnessState::automatic),
      static_cast<std::uint8_t>(status.state));
}

void test_invalid_frame_breaks_end_to_end_fallback_recovery() {
  CivicAuxReceiver receiver;
  AutomaticBrightnessController controller;
  controller.reset(BrightnessMode::automatic,
                   20,
                   kFullAutomaticMinimumPercent,
                   kFullAutomaticMaximumPercent,
                   0);

  feedReceiver(receiver,
               makeAmbient(1, 100, CivicAuxSensorState::valid,
                           kCivicAuxFlagDataValid, 0, 30'000'000),
               100);
  (void)controller.update(receiver.snapshot(), 100);
  feedReceiver(receiver,
               makeAmbient(2, 300, CivicAuxSensorState::valid,
                           kCivicAuxFlagDataValid, 0, 30'000'000),
               300);
  auto status = controller.update(receiver.snapshot(), 300);
  TEST_ASSERT_EQUAL_UINT8(100, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(28, status.appliedPercent);

  auto corrupt = makeAmbient(3, 500, CivicAuxSensorState::valid,
                             kCivicAuxFlagDataValid, 0, 30'000'000);
  corrupt.bytes[corrupt.length - 1U] ^= 0x01U;
  feedReceiver(receiver, corrupt, 500);
  feedReceiver(receiver, corrupt, 1500);
  status = controller.update(receiver.snapshot(), 1500);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(AutomaticBrightnessState::fallback),
      static_cast<std::uint8_t>(status.state));

  feedReceiver(receiver,
               makeAmbient(4, 1600, CivicAuxSensorState::valid,
                           kCivicAuxFlagDataValid, 0, 500'000),
               1600);
  status = controller.update(receiver.snapshot(), 1600);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(AutomaticBrightnessState::fallback),
      static_cast<std::uint8_t>(status.state));

  corrupt = makeAmbient(5, 1700, CivicAuxSensorState::valid,
                        kCivicAuxFlagDataValid, 0, 500'000);
  corrupt.bytes[corrupt.length - 1U] ^= 0x01U;
  feedReceiver(receiver, corrupt, 1700);
  feedReceiver(receiver,
               makeAmbient(6, 1800, CivicAuxSensorState::valid,
                           kCivicAuxFlagDataValid, 0, 500'000),
               1800);
  status = controller.update(receiver.snapshot(), 1800);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(AutomaticBrightnessState::fallback),
      static_cast<std::uint8_t>(status.state));

  feedReceiver(receiver,
               makeAmbient(7, 2000, CivicAuxSensorState::valid,
                           kCivicAuxFlagDataValid, 0, 500'000),
               2000);
  status = controller.update(receiver.snapshot(), 2000);
  TEST_ASSERT_EQUAL_UINT8(50, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(34, status.appliedPercent);
  status = controller.update(receiver.snapshot(), 2400);
  TEST_ASSERT_EQUAL_UINT8(50, status.appliedPercent);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(AutomaticBrightnessState::automatic),
      static_cast<std::uint8_t>(status.state));
}

void test_menu_and_warning_preferences_do_not_change_automatic_brightness() {
  GaugeSettings settings{};
  settings.brightnessMode = BrightnessMode::automatic;
  settings.brightnessPercent = 35;
  AutomaticBrightnessController controller;
  controller.reset(
      settings.brightnessMode,
      settings.brightnessPercent,
      settings.automaticBrightnessMinimumPercent,
      settings.automaticBrightnessMaximumPercent,
      0);

  (void)controller.update(
      usableSnapshot(1, 1, 10'000'000, 100, 5000), 100);
  const CivicAuxSnapshot active =
      usableSnapshot(2, 2, 10'000'000, 300, 5000);
  auto status = controller.update(active, 300);
  TEST_ASSERT_EQUAL_UINT8(87, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(43, status.appliedPercent);

  settings.warningVisualMode = WarningVisualMode::fullScreenBlink;
  settings.warningSoundEnabled = false;
  settings.warningVolumePercent = 100;
  settings.lowPressureWarningPsi = 25;
  settings.highTemperatureWarningCelsius = 135;
  settings.pressureUnit = PressureUnit::bar;
  settings.temperatureUnit = TemperatureUnit::fahrenheit;
  settings.dataSource = DataSource::sensors;
  controller.setPreferences(
      settings.brightnessMode,
      settings.brightnessPercent,
      settings.automaticBrightnessMinimumPercent,
      settings.automaticBrightnessMaximumPercent,
      400);
  status = controller.update(active, 400);

  TEST_ASSERT_EQUAL_UINT8(87, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(47, status.appliedPercent);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<std::uint8_t>(AutomaticBrightnessState::automatic),
      static_cast<std::uint8_t>(status.state));
  TEST_ASSERT_FALSE(status.persistenceRequested);
}

void test_automatic_limits_compress_the_full_curve_between_both_ends() {
  AutomaticBrightnessController controller;
  controller.reset(BrightnessMode::automatic, 40, 40, 75, 0);

  (void)controller.update(usableSnapshot(1, 1, 0, 100, 5000), 100);
  auto status =
      controller.update(usableSnapshot(2, 2, 0, 300, 5000), 300);
  TEST_ASSERT_EQUAL_UINT8(40, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(40, status.appliedPercent);

  status = controller.update(
      usableSnapshot(3, 3, 500'000, 500, 5000), 500);
  TEST_ASSERT_EQUAL_UINT8(57, status.automaticPercent);

  status = controller.update(
      usableSnapshot(4, 4, 30'000'000, 700, 5000), 700);
  TEST_ASSERT_EQUAL_UINT8(75, status.automaticPercent);
}

void test_automatic_limits_reapply_without_a_new_lux_frame() {
  AutomaticBrightnessController controller;
  controller.reset(BrightnessMode::automatic,
                   55,
                   kFullAutomaticMinimumPercent,
                   kFullAutomaticMaximumPercent,
                   0);
  (void)controller.update(
      usableSnapshot(1, 1, 30'000'000, 100, 5000), 100);
  const CivicAuxSnapshot active =
      usableSnapshot(2, 2, 30'000'000, 300, 5000);
  auto status = controller.update(active, 300);
  TEST_ASSERT_EQUAL_UINT8(100, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(63, status.appliedPercent);
  status = controller.update(active, 1225);
  TEST_ASSERT_EQUAL_UINT8(100, status.appliedPercent);

  controller.setPreferences(BrightnessMode::automatic, 55, 20, 70, 1300);
  status = controller.update(active, 1300);
  TEST_ASSERT_EQUAL_UINT8(70, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(98, status.appliedPercent);
  status = controller.update(active, 2420);
  TEST_ASSERT_EQUAL_UINT8(70, status.appliedPercent);
}

void test_automatic_curve_adjustment_survives_a_raised_minimum() {
  AutomaticBrightnessController controller;
  controller.reset(BrightnessMode::automatic, 55, 40, 100, 0, 0);
  (void)controller.update(
      usableSnapshot(1, 1, 100'000, 100, 5000), 100);
  const CivicAuxSnapshot active =
      usableSnapshot(2, 2, 100'000, 300, 5000);
  auto status = controller.update(active, 300);
  TEST_ASSERT_EQUAL_UINT8(59, status.automaticPercent);

  controller.setPreferences(
      BrightnessMode::automatic, 55, 40, 100, 400, 30);
  status = controller.update(active, 400);
  TEST_ASSERT_EQUAL_UINT8(81, status.automaticPercent);

  controller.setPreferences(
      BrightnessMode::automatic, 55, 40, 100, 500, -30);
  status = controller.update(active, 500);
  TEST_ASSERT_EQUAL_UINT8(42, status.automaticPercent);

  controller.setPreferences(
      BrightnessMode::automatic, 55, 40, 100, 600, 0);
  status = controller.update(active, 600);
  TEST_ASSERT_EQUAL_UINT8(59, status.automaticPercent);
}

void test_automatic_brightness_uses_a_time_based_asymmetric_ramp() {
  AutomaticBrightnessController controller;
  controller.reset(BrightnessMode::automatic,
                   20,
                   20,
                   kFullAutomaticMaximumPercent,
                   0);
  (void)controller.update(
      usableSnapshot(1, 1, 30'000'000, 100, 10'000), 100);
  const CivicAuxSnapshot bright =
      usableSnapshot(2, 2, 30'000'000, 200, 10'000);
  auto status = controller.update(bright, 200);
  TEST_ASSERT_EQUAL_UINT8(24, status.appliedPercent);
  status = controller.update(bright, 1200);
  TEST_ASSERT_EQUAL_UINT8(64, status.appliedPercent);
  status = controller.update(bright, 2100);
  TEST_ASSERT_EQUAL_UINT8(100, status.appliedPercent);

  const CivicAuxSnapshot dark =
      usableSnapshot(3, 3, 0, 2200, 10'000);
  status = controller.update(dark, 2200);
  TEST_ASSERT_EQUAL_UINT8(20, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(98, status.appliedPercent);
  status = controller.update(dark, 3200);
  TEST_ASSERT_EQUAL_UINT8(73, status.appliedPercent);
  status = controller.update(dark, 5300);
  TEST_ASSERT_EQUAL_UINT8(20, status.appliedPercent);
}

void test_one_percent_lux_jitter_does_not_toggle_the_auto_target() {
  TEST_ASSERT_EQUAL_UINT8(
      34, automaticBrightnessPercentForMillilux(90'000));
  TEST_ASSERT_EQUAL_UINT8(
      35, automaticBrightnessPercentForMillilux(100'000));

  AutomaticBrightnessController controller;
  controller.reset(BrightnessMode::automatic,
                   34,
                   kFullAutomaticMinimumPercent,
                   kFullAutomaticMaximumPercent,
                   0);
  (void)controller.update(
      usableSnapshot(1, 1, 90'000, 100, 5000), 100);
  auto status = controller.update(
      usableSnapshot(2, 2, 90'000, 300, 5000), 300);
  TEST_ASSERT_EQUAL_UINT8(34, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(34, status.appliedPercent);

  status = controller.update(
      usableSnapshot(3, 3, 100'000, 500, 5000), 500);
  TEST_ASSERT_EQUAL_UINT8(34, status.automaticPercent);
  TEST_ASSERT_EQUAL_UINT8(34, status.appliedPercent);
}

}  // namespace

void setUp() {}
void tearDown() {}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_crc_standard_check_vector);
  RUN_TEST(test_hub_status_vector_is_byte_exact_and_little_endian);
  RUN_TEST(test_ambient_vector_is_byte_exact_and_little_endian);
  RUN_TEST(test_parser_resynchronizes_across_noise_between_vectors);
  RUN_TEST(test_truncated_frame_followed_by_valid_frame_resynchronizes);
  RUN_TEST(test_bad_crc_is_rejected_before_next_valid_frame);
  RUN_TEST(test_incompatible_version_is_rejected);
  RUN_TEST(test_payload_over_48_is_rejected_and_parser_recovers);
  RUN_TEST(test_wrong_ambient_length_and_wrong_target_are_rejected);
  RUN_TEST(test_unknown_valid_type_is_consumed_and_ignored);
  RUN_TEST(test_flags_state_and_communicated_freshness_gate_lux);
  RUN_TEST(test_hub_status_never_renews_lux_freshness);
  RUN_TEST(test_sequence_wrap_is_continuous_but_hub_restart_resets_recovery);
  RUN_TEST(test_invalid_frame_breaks_two_sample_recovery_sequence);
  RUN_TEST(test_continued_invalid_traffic_arms_one_second_fallback);
  RUN_TEST(test_brightness_curve_exact_points_and_intermediate_values);
  RUN_TEST(test_brightness_curve_is_monotonic_and_saturates);
  RUN_TEST(test_brightness_mode_defaults_migrates_and_round_trips);
  RUN_TEST(test_auto_starts_on_backup_and_requires_two_new_samples);
  RUN_TEST(test_manual_mode_ignores_all_lux_and_uses_backup_slider);
  RUN_TEST(test_invalid_traffic_falls_back_smoothly_over_1500_ms);
  RUN_TEST(test_two_second_timeout_falls_back_and_two_samples_recover);
  RUN_TEST(test_hub_restart_requires_two_post_restart_samples);
  RUN_TEST(test_automatic_samples_never_request_settings_persistence);
  RUN_TEST(test_communicated_age_shortens_the_local_freshness_window);
  RUN_TEST(test_manual_to_auto_requires_two_samples_received_after_selection);
  RUN_TEST(test_invalid_frame_breaks_end_to_end_fallback_recovery);
  RUN_TEST(
      test_menu_and_warning_preferences_do_not_change_automatic_brightness);
  RUN_TEST(test_automatic_limits_compress_the_full_curve_between_both_ends);
  RUN_TEST(test_automatic_limits_reapply_without_a_new_lux_frame);
  RUN_TEST(test_automatic_curve_adjustment_survives_a_raised_minimum);
  RUN_TEST(test_automatic_brightness_uses_a_time_based_asymmetric_ramp);
  RUN_TEST(test_one_percent_lux_jitter_does_not_toggle_the_auto_target);
  return UNITY_END();
}
