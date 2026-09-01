#include "oil_gauge_ui.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <iterator>

#include "oil_gauge_fonts.h"
#include "icons/oil_icon_assets.h"
namespace oilgauge {

namespace {

constexpr std::int32_t kCanvasWidth = 480;
constexpr std::int32_t kHalfHeight = 240;
constexpr std::int32_t kContentX = 38;
constexpr std::int32_t kContentWidth = 404;
constexpr std::int32_t kBarHeight = 21;

constexpr std::uint32_t kBlack = 0x000000;
constexpr std::uint32_t kPrimary = 0xF7F9FB;
constexpr std::uint32_t kSecondary = 0x9AA4AF;
constexpr std::uint32_t kLine = 0x262728;
constexpr std::uint32_t kPanel = 0x151719;
constexpr std::uint32_t kPanelSelected = 0x394047;
constexpr std::uint32_t kWarningRed = 0xFF3948;

struct BarWidgets {
  lv_obj_t* track = nullptr;
  lv_obj_t* fill = nullptr;
  lv_obj_t* firstTick = nullptr;
  std::int32_t width = -1;
  RgbColor fillColor{};
  lv_opa_t opacity = LV_OPA_TRANSP;
  bool colorSet = false;
  bool hidden = false;
};

enum class MenuPage : std::size_t {
  home = 0,
  brightness,
  data,
  warnings,
  sound,
  units,
  startup,
  system,
  count,
};

struct UiWidgets {
  lv_obj_t* screen = nullptr;
  lv_obj_t* gaugeRoot = nullptr;
  lv_obj_t* menu = nullptr;
  lv_obj_t* resetConfirm = nullptr;
  lv_obj_t* fullScreenWarning = nullptr;
  lv_obj_t* fullScreenPressureValue = nullptr;
  lv_obj_t* bootSplash = nullptr;
  std::array<lv_obj_t*, static_cast<std::size_t>(MenuPage::count)> menuPages{};
  lv_obj_t* homeBrightnessSummary = nullptr;
  lv_obj_t* homeDataSummary = nullptr;
  lv_obj_t* homeWarningsSummary = nullptr;
  lv_obj_t* homeSoundSummary = nullptr;
  lv_obj_t* homeUnitsSummary = nullptr;
  lv_obj_t* homeStartupSummary = nullptr;
  lv_obj_t* pressureState = nullptr;
  lv_obj_t* sourceBadge = nullptr;
  lv_obj_t* pressureValue = nullptr;
  lv_obj_t* pressureUnit = nullptr;
  lv_obj_t* pressureIcon = nullptr;
  BarWidgets pressureBar{};
  lv_obj_t* temperatureState = nullptr;
  lv_obj_t* temperatureValue = nullptr;
  lv_obj_t* temperatureUnit = nullptr;
  lv_obj_t* temperatureIcon = nullptr;
  RgbColor pressureIconColor{};
  bool pressureIconColorSet = false;
  RgbColor temperatureIconColor{};
  bool temperatureIconColorSet = false;
  lv_obj_t* brightnessSlider = nullptr;
  lv_obj_t* brightnessValue = nullptr;
  lv_obj_t* automaticBrightnessRangeSlider = nullptr;
  lv_obj_t* automaticBrightnessRangeValue = nullptr;
  lv_obj_t* brightnessAutoButton = nullptr;
  lv_obj_t* brightnessManualButton = nullptr;
  lv_obj_t* ambientLuxLabel = nullptr;
  lv_obj_t* ambientStateLabel = nullptr;
  lv_obj_t* automaticBrightnessLabel = nullptr;
  lv_obj_t* soundSwitch = nullptr;
  lv_obj_t* volumeSlider = nullptr;
  lv_obj_t* volumeValue = nullptr;
  lv_obj_t* pressureWarningSlider = nullptr;
  lv_obj_t* pressureWarningValue = nullptr;
  lv_obj_t* temperatureWarningSlider = nullptr;
  lv_obj_t* temperatureWarningValue = nullptr;
  lv_obj_t* startupLogoSlider = nullptr;
  lv_obj_t* startupLogoValue = nullptr;
  lv_obj_t* demoButton = nullptr;
  lv_obj_t* sensorsButton = nullptr;
  lv_obj_t* unitPsiButton = nullptr;
  lv_obj_t* unitBarButton = nullptr;
  lv_obj_t* unitCelsiusButton = nullptr;
  lv_obj_t* unitFahrenheitButton = nullptr;
  lv_obj_t* warningElementsButton = nullptr;
  lv_obj_t* warningScreenButton = nullptr;
  lv_obj_t* warningFixedButton = nullptr;
  lv_obj_t* warningModeDescription = nullptr;
  BarWidgets temperatureBar{};
  char pressureValueText[8]{};
  char sourceBadgeText[16]{};
  char fullScreenPressureValueText[8]{};
  bool temperatureAttentionHidden = false;
  char pressureStateText[24]{};
  char temperatureValueText[8]{};
  char temperatureStateText[24]{};
  char pressureWarningText[24]{};
  char temperatureWarningText[24]{};
  char startupLogoText[24]{};
  char brightnessValueText[40]{};
  char volumeValueText[24]{};
  char warningModeDescriptionText[48]{};
  char homeBrightnessSummaryText[40]{};
  char homeDataSummaryText[24]{};
  char homeWarningsSummaryText[48]{};
  char homeSoundSummaryText[32]{};
  char homeUnitsSummaryText[24]{};
  char homeStartupSummaryText[32]{};
  char automaticBrightnessRangeText[40]{};
  char ambientLuxText[40]{};
  char ambientStateText[56]{};
  char automaticBrightnessText[56]{};
  RgbColor pressureColor{};
  RgbColor temperatureColor{};
  GaugeSettings settings{};
  GaugeSettings defaults{};
  OilGaugeBrightnessStatus brightnessStatus{};
  PressureUnit renderedUnit = PressureUnit::psi;
  TemperatureUnit renderedTemperatureUnit = TemperatureUnit::celsius;
  OilGaugeUiActions pendingActions{};
  lv_opa_t pressureAttentionOpacity = LV_OPA_TRANSP;
  bool pressureColorSet = false;
  bool temperatureColorSet = false;
  bool menuVisible = false;
  bool menuDirty = false;
  bool warningActive = false;
  bool fullScreenWarningVisible = false;
  bool bootSplashVisible = false;
  bool unitRendered = false;
  bool temperatureUnitRendered = false;
  bool actionsPending = false;
  bool created = false;
  MenuPage activeMenuPage = MenuPage::home;
};

UiWidgets gUi;

void setFullScreenWarningVisible(bool visible) {
  if (gUi.fullScreenWarningVisible != visible) {
    if (visible) {
      lv_obj_clear_flag(gUi.fullScreenWarning, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(gUi.fullScreenWarning, LV_OBJ_FLAG_HIDDEN);
    }
    gUi.fullScreenWarningVisible = visible;
  }
}

lv_color_t color(std::uint32_t rgb) {
  return lv_color_hex(rgb);
}

lv_color_t color(const RgbColor& rgb) {
  return lv_color_make(rgb.red, rgb.green, rgb.blue);
}

lv_obj_t* createLabel(lv_obj_t* parent,
                      const char* text,
                      std::int32_t x,
                      std::int32_t y,
                      std::int32_t width,
                      std::int32_t height,
                      const lv_font_t* font,
                      lv_color_t textColor,
                      lv_text_align_t alignment) {
  lv_obj_t* label = lv_label_create(parent);
  lv_obj_remove_style_all(label);
  lv_obj_set_pos(label, x, y);
  lv_obj_set_size(label, width, height);
  lv_obj_set_style_text_font(label, font, 0);
  lv_obj_set_style_text_color(label, textColor, 0);
  lv_obj_set_style_text_align(label, alignment, 0);
  lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
  lv_label_set_text(label, text);
  return label;
}

lv_obj_t* createSolid(lv_obj_t* parent,
                      std::int32_t x,
                      std::int32_t y,
                      std::int32_t width,
                      std::int32_t height,
                      std::int32_t radius) {
  lv_obj_t* object = lv_obj_create(parent);
  lv_obj_remove_style_all(object);
  lv_obj_set_pos(object, x, y);
  lv_obj_set_size(object, width, height);
  lv_obj_set_style_radius(object, radius, 0);
  lv_obj_set_style_bg_opa(object, LV_OPA_COVER, 0);
  lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
  return object;
}

void setIconColor(lv_obj_t* icon, lv_color_t iconColor) {
  lv_obj_set_style_image_recolor(icon, iconColor, 0);
  lv_obj_set_style_image_recolor_opa(icon, LV_OPA_COVER, 0);
  const std::uint32_t childCount = lv_obj_get_child_count(icon);
  for (std::uint32_t index = 0; index < childCount; ++index) {
    lv_obj_t* child = lv_obj_get_child(icon, static_cast<std::int32_t>(index));
    lv_obj_set_style_line_color(child, iconColor, 0);
    lv_obj_set_style_bg_color(child, iconColor, 0);
  }
}

bool sameColor(const RgbColor& left, const RgbColor& right) {
  return left.red == right.red && left.green == right.green &&
         left.blue == right.blue;
}

template <std::size_t Size>
void setLabelTextIfChanged(lv_obj_t* label,
                           char (&previous)[Size],
                           const char* text) {
  if (std::strcmp(previous, text) == 0) {
    return;
  }
  lv_label_set_text(label, text);
  std::snprintf(previous, Size, "%s", text);
}

void setChoiceSelected(lv_obj_t* button, bool selected) {
  lv_obj_set_style_bg_color(
      button, color(selected ? kPanelSelected : kPanel), 0);
  lv_obj_set_style_border_color(
      button, color(selected ? kPrimary : kLine), 0);
}

const char* ambientSensorStateName(std::uint8_t state) {
  switch (static_cast<CivicAuxSensorState>(state)) {
    case CivicAuxSensorState::initializing:
      return "INICIANDO";
    case CivicAuxSensorState::valid:
      return "VÁLIDO";
    case CivicAuxSensorState::degraded:
      return "DEGRADADO";
    case CivicAuxSensorState::missing:
      return "AUSENTE";
  }
  return "DESCONOCIDO";
}

const char* ambientRangeName(std::uint8_t range) {
  switch (static_cast<CivicAuxRangeProfile>(range)) {
    case CivicAuxRangeProfile::dark:
      return "OSCURO";
    case CivicAuxRangeProfile::normal:
      return "NORMAL";
    case CivicAuxRangeProfile::intense:
      return "INTENSO";
  }
  return "RANGO ?";
}

const char* automaticBrightnessStateName(AutomaticBrightnessState state) {
  switch (state) {
    case AutomaticBrightnessState::manual:
      return "MANUAL";
    case AutomaticBrightnessState::waitingForSamples:
      return "ESPERANDO";
    case AutomaticBrightnessState::automatic:
      return "ACTIVO";
    case AutomaticBrightnessState::fallback:
      return "FALLBACK";
  }
  return "ESPERANDO";
}

bool sameBrightnessStatus(const OilGaugeBrightnessStatus& left,
                          const OilGaugeBrightnessStatus& right) {
  return left.receiverRunning == right.receiverRunning &&
         left.hasAmbientFrame == right.hasAmbientFrame &&
         left.latestAmbientUsable == right.latestAmbientUsable &&
         left.luxFresh == right.luxFresh &&
         left.sensorState == right.sensorState &&
         left.rangeProfile == right.rangeProfile &&
         left.filteredMillilux == right.filteredMillilux &&
         left.automaticState == right.automaticState &&
         left.automaticPercent == right.automaticPercent &&
         left.appliedPercent == right.appliedPercent &&
         left.automaticPercentAvailable ==
             right.automaticPercentAvailable;
}

void refreshBrightnessTelemetry() {
  if (gUi.ambientLuxLabel == nullptr) {
    return;
  }
  if (!gUi.brightnessStatus.hasAmbientFrame) {
    setLabelTextIfChanged(
        gUi.ambientLuxLabel, gUi.ambientLuxText, "LUZ HUB: --");
  } else {
    char luxText[40];
    std::snprintf(luxText,
                  sizeof(luxText),
                  "LUZ HUB: %lu.%01lu LX",
                  static_cast<unsigned long>(
                      gUi.brightnessStatus.filteredMillilux / 1000U),
                  static_cast<unsigned long>(
                      (gUi.brightnessStatus.filteredMillilux % 1000U) / 100U));
    setLabelTextIfChanged(
        gUi.ambientLuxLabel, gUi.ambientLuxText, luxText);
  }

  char stateText[56];
  if (!gUi.brightnessStatus.receiverRunning) {
    std::snprintf(stateText, sizeof(stateText), "UART: NO DISPONIBLE");
  } else if (!gUi.brightnessStatus.hasAmbientFrame) {
    std::snprintf(stateText, sizeof(stateText), "UART: ESPERANDO LUZ");
  } else {
    std::snprintf(
        stateText,
        sizeof(stateText),
        "%s · %s · %s",
        ambientSensorStateName(gUi.brightnessStatus.sensorState),
        ambientRangeName(gUi.brightnessStatus.rangeProfile),
        gUi.brightnessStatus.latestAmbientUsable &&
                gUi.brightnessStatus.luxFresh
            ? "FRESCO"
            : "NO UTILIZABLE");
  }
  setLabelTextIfChanged(
      gUi.ambientStateLabel, gUi.ambientStateText, stateText);

  char automaticText[56];
  if (gUi.brightnessStatus.automaticPercentAvailable) {
    std::snprintf(
        automaticText,
        sizeof(automaticText),
        "AUTO %s: %u%% · APLICADO: %u%%",
        automaticBrightnessStateName(gUi.brightnessStatus.automaticState),
        static_cast<unsigned>(gUi.brightnessStatus.automaticPercent),
        static_cast<unsigned>(gUi.brightnessStatus.appliedPercent));
  } else {
    std::snprintf(
        automaticText,
        sizeof(automaticText),
        "AUTO %s · APLICADO: %u%%",
        automaticBrightnessStateName(gUi.brightnessStatus.automaticState),
        static_cast<unsigned>(gUi.brightnessStatus.appliedPercent));
  }
  setLabelTextIfChanged(gUi.automaticBrightnessLabel,
                        gUi.automaticBrightnessText,
                        automaticText);
}

void refreshAutomaticBrightnessRangeLabel() {
  if (gUi.automaticBrightnessRangeValue == nullptr) {
    return;
  }
  char rangeText[40];
  std::snprintf(
      rangeText,
      sizeof(rangeText),
      "LÍMITES AUTO: %u–%u%%",
      static_cast<unsigned>(gUi.settings.automaticBrightnessMinimumPercent),
      static_cast<unsigned>(gUi.settings.automaticBrightnessMaximumPercent));
  setLabelTextIfChanged(gUi.automaticBrightnessRangeValue,
                        gUi.automaticBrightnessRangeText,
                        rangeText);
}

void refreshWarningModeDescription() {
  const char* description = "ELEMENTOS · 2 HZ";
  if (gUi.settings.warningVisualMode == WarningVisualMode::fullScreenBlink) {
    description = "PANTALLA ROJA · 0,5 HZ";
  } else if (gUi.settings.warningVisualMode == WarningVisualMode::fixed) {
    description = "AVISO FIJO · SIN PARPADEO";
  }
  setLabelTextIfChanged(gUi.warningModeDescription,
                        gUi.warningModeDescriptionText,
                        description);
}

void refreshMenuSummaries() {
  if (gUi.homeBrightnessSummary == nullptr) {
    return;
  }

  char brightnessText[40];
  if (gUi.settings.brightnessMode == BrightnessMode::automatic) {
    std::snprintf(
        brightnessText,
        sizeof(brightnessText),
        "AUTO · %u–%u%%",
        static_cast<unsigned>(gUi.settings.automaticBrightnessMinimumPercent),
        static_cast<unsigned>(gUi.settings.automaticBrightnessMaximumPercent));
  } else {
    std::snprintf(brightnessText,
                  sizeof(brightnessText),
                  "MANUAL · %u%%",
                  static_cast<unsigned>(gUi.settings.brightnessPercent));
  }
  setLabelTextIfChanged(gUi.homeBrightnessSummary,
                        gUi.homeBrightnessSummaryText,
                        brightnessText);

  setLabelTextIfChanged(
      gUi.homeDataSummary,
      gUi.homeDataSummaryText,
      gUi.settings.dataSource == DataSource::demo ? "DEMO" : "SENSORES");

  const bool bar = gUi.settings.pressureUnit == PressureUnit::bar;
  const double pressureThreshold = warningThresholdForDisplay(
      gUi.settings.lowPressureWarningPsi, gUi.settings.pressureUnit);
  const double temperatureThreshold = temperatureWarningThresholdForDisplay(
      gUi.settings.highTemperatureWarningCelsius,
      gUi.settings.temperatureUnit);
  char warningsText[48];
  if (bar) {
    std::snprintf(warningsText,
                  sizeof(warningsText),
                  "%.1f BAR · %.0f °%c",
                  pressureThreshold,
                  temperatureThreshold,
                  gUi.settings.temperatureUnit == TemperatureUnit::fahrenheit
                      ? 'F'
                      : 'C');
  } else {
    std::snprintf(warningsText,
                  sizeof(warningsText),
                  "%u PSI · %.0f °%c",
                  static_cast<unsigned>(gUi.settings.lowPressureWarningPsi),
                  temperatureThreshold,
                  gUi.settings.temperatureUnit == TemperatureUnit::fahrenheit
                      ? 'F'
                      : 'C');
  }
  setLabelTextIfChanged(gUi.homeWarningsSummary,
                        gUi.homeWarningsSummaryText,
                        warningsText);

  char soundText[32];
  if (gUi.settings.warningSoundEnabled) {
    std::snprintf(soundText,
                  sizeof(soundText),
                  "ACTIVO · %u%%",
                  static_cast<unsigned>(gUi.settings.warningVolumePercent));
  } else {
    std::snprintf(soundText, sizeof(soundText), "DESACTIVADO");
  }
  setLabelTextIfChanged(
      gUi.homeSoundSummary, gUi.homeSoundSummaryText, soundText);

  char unitsText[24];
  std::snprintf(
      unitsText,
      sizeof(unitsText),
      "%s · °%c",
      bar ? "BAR" : "PSI",
      gUi.settings.temperatureUnit == TemperatureUnit::fahrenheit ? 'F' : 'C');
  setLabelTextIfChanged(
      gUi.homeUnitsSummary, gUi.homeUnitsSummaryText, unitsText);

  char startupText[32];
  if (gUi.settings.startupLogoSeconds == 0) {
    std::snprintf(startupText, sizeof(startupText), "DESACTIVADO");
  } else {
    std::snprintf(startupText,
                  sizeof(startupText),
                  "LOGO · %u S",
                  static_cast<unsigned>(gUi.settings.startupLogoSeconds));
  }
  setLabelTextIfChanged(gUi.homeStartupSummary,
                        gUi.homeStartupSummaryText,
                        startupText);
}

void refreshBrightnessValueLabel() {
  char brightnessValueText[40];
  std::snprintf(
      brightnessValueText,
      sizeof(brightnessValueText),
      gUi.settings.brightnessMode == BrightnessMode::automatic
          ? "RESPALDO MANUAL: %u%%"
          : "BRILLO MANUAL: %u%%",
      static_cast<unsigned>(gUi.settings.brightnessPercent));
  setLabelTextIfChanged(gUi.brightnessValue,
                        gUi.brightnessValueText,
                        brightnessValueText);
}

void refreshVolumeValueLabel() {
  char volumeText[24];
  std::snprintf(volumeText,
                sizeof(volumeText),
                "VOLUMEN: %u%%",
                static_cast<unsigned>(gUi.settings.warningVolumePercent));
  setLabelTextIfChanged(
      gUi.volumeValue, gUi.volumeValueText, volumeText);
}

void refreshMenuControls() {
  if (gUi.brightnessSlider == nullptr) {
    return;
  }
  lv_slider_set_value(
      gUi.brightnessSlider, gUi.settings.brightnessPercent, LV_ANIM_OFF);
  refreshBrightnessValueLabel();
  lv_slider_set_left_value(
      gUi.automaticBrightnessRangeSlider,
      gUi.settings.automaticBrightnessMinimumPercent,
      LV_ANIM_OFF);
  lv_slider_set_value(
      gUi.automaticBrightnessRangeSlider,
      gUi.settings.automaticBrightnessMaximumPercent,
      LV_ANIM_OFF);
  refreshAutomaticBrightnessRangeLabel();
  lv_slider_set_value(
      gUi.volumeSlider, gUi.settings.warningVolumePercent, LV_ANIM_OFF);
  refreshVolumeValueLabel();
  const bool bar = gUi.settings.pressureUnit == PressureUnit::bar;
  lv_slider_set_range(gUi.pressureWarningSlider, 1, bar ? 21 : 30);
  const double threshold = warningThresholdForDisplay(
      gUi.settings.lowPressureWarningPsi, gUi.settings.pressureUnit);
  if (gUi.pressureBar.firstTick != nullptr) {
    const std::int32_t warningTickX =
        kContentX + static_cast<std::int32_t>(std::lround(
                        gUi.settings.lowPressureWarningPsi / 85.0 *
                        static_cast<double>(kContentWidth)));
    lv_obj_set_x(gUi.pressureBar.firstTick, warningTickX);
  }
  lv_slider_set_value(gUi.pressureWarningSlider,
                      bar ? static_cast<std::int32_t>(std::lround(threshold * 10.0))
                          : gUi.settings.lowPressureWarningPsi,
                      LV_ANIM_OFF);
  char thresholdText[24];
  if (bar) {
    std::snprintf(thresholdText, sizeof(thresholdText), "UMBRAL: %.1f BAR", threshold);
  } else {
    std::snprintf(thresholdText,
                  sizeof(thresholdText),
                  "UMBRAL: %u PSI",
                  static_cast<unsigned>(gUi.settings.lowPressureWarningPsi));
  }
  setLabelTextIfChanged(
      gUi.pressureWarningValue, gUi.pressureWarningText, thresholdText);
  if (gUi.temperatureBar.firstTick != nullptr) {
    const double temperatureRatio =
        (static_cast<double>(gUi.settings.highTemperatureWarningCelsius) -
         50.0) /
        90.0;
    const std::int32_t warningTickX =
        kContentX + static_cast<std::int32_t>(std::lround(
                        temperatureRatio *
                        static_cast<double>(kContentWidth)));
    lv_obj_set_x(gUi.temperatureBar.firstTick, warningTickX);
  }
  const bool fahrenheit =
      gUi.settings.temperatureUnit == TemperatureUnit::fahrenheit;
  const double temperatureThreshold = temperatureWarningThresholdForDisplay(
      gUi.settings.highTemperatureWarningCelsius,
      gUi.settings.temperatureUnit);
  lv_slider_set_range(gUi.temperatureWarningSlider,
                      fahrenheit ? 230 : 110,
                      fahrenheit ? 284 : 140);
  lv_slider_set_value(gUi.temperatureWarningSlider,
                      static_cast<std::int32_t>(
                          std::lround(temperatureThreshold)),
                      LV_ANIM_OFF);
  char temperatureThresholdText[24];
  std::snprintf(
      temperatureThresholdText,
      sizeof(temperatureThresholdText),
      "UMBRAL: %.0f °%c",
      temperatureThreshold,
      fahrenheit ? 'F' : 'C');
  setLabelTextIfChanged(gUi.temperatureWarningValue,
                        gUi.temperatureWarningText,
                        temperatureThresholdText);
  lv_slider_set_value(gUi.startupLogoSlider,
                      gUi.settings.startupLogoSeconds,
                      LV_ANIM_OFF);
  char startupText[24];
  if (gUi.settings.startupLogoSeconds == 0) {
    std::snprintf(startupText, sizeof(startupText), "LOGOTIPO DESACTIVADO");
  } else {
    std::snprintf(startupText,
                  sizeof(startupText),
                  "DURACIÓN: %u S",
                  static_cast<unsigned>(gUi.settings.startupLogoSeconds));
  }
  setLabelTextIfChanged(
      gUi.startupLogoValue, gUi.startupLogoText, startupText);
  if (gUi.settings.warningSoundEnabled) {
    lv_obj_add_state(gUi.soundSwitch, LV_STATE_CHECKED);
  } else {
    lv_obj_remove_state(gUi.soundSwitch, LV_STATE_CHECKED);
  }
  setChoiceSelected(
      gUi.unitPsiButton, gUi.settings.pressureUnit == PressureUnit::psi);
  setChoiceSelected(
      gUi.unitBarButton, gUi.settings.pressureUnit == PressureUnit::bar);
  setChoiceSelected(gUi.unitCelsiusButton,
                    gUi.settings.temperatureUnit == TemperatureUnit::celsius);
  setChoiceSelected(
      gUi.unitFahrenheitButton,
      gUi.settings.temperatureUnit == TemperatureUnit::fahrenheit);
  setChoiceSelected(
      gUi.demoButton, gUi.settings.dataSource == DataSource::demo);
  setChoiceSelected(
      gUi.sensorsButton, gUi.settings.dataSource == DataSource::sensors);
  setChoiceSelected(
      gUi.brightnessAutoButton,
      gUi.settings.brightnessMode == BrightnessMode::automatic);
  setChoiceSelected(
      gUi.brightnessManualButton,
      gUi.settings.brightnessMode == BrightnessMode::manual);
  setChoiceSelected(
      gUi.warningElementsButton,
      gUi.settings.warningVisualMode == WarningVisualMode::elementsBlink);
  setChoiceSelected(
      gUi.warningScreenButton,
      gUi.settings.warningVisualMode == WarningVisualMode::fullScreenBlink);
  setChoiceSelected(
      gUi.warningFixedButton,
      gUi.settings.warningVisualMode == WarningVisualMode::fixed);
  refreshWarningModeDescription();
  refreshMenuSummaries();
}

void queueSettingsApply() {
  gUi.settings = sanitizeGaugeSettings(gUi.settings);
  gUi.pendingActions.settings = gUi.settings;
  gUi.pendingActions.applySettings = true;
  gUi.actionsPending = true;
  gUi.menuDirty = true;
}

void closeMenu(bool save) {
  if (!gUi.menuVisible) {
    return;
  }
  lv_obj_add_flag(gUi.menu, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(gUi.resetConfirm, LV_OBJ_FLAG_HIDDEN);
  gUi.menuVisible = false;
  if (save && gUi.menuDirty) {
    gUi.pendingActions.settings = gUi.settings;
    gUi.pendingActions.saveSettings = true;
    gUi.actionsPending = true;
    gUi.menuDirty = false;
  }
}

void showMenuPage(MenuPage page) {
  for (lv_obj_t* menuPage : gUi.menuPages) {
    if (menuPage != nullptr) {
      lv_obj_add_flag(menuPage, LV_OBJ_FLAG_HIDDEN);
    }
  }
  lv_obj_t* selected =
      gUi.menuPages[static_cast<std::size_t>(page)];
  if (selected == nullptr) {
    return;
  }
  lv_obj_clear_flag(selected, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(selected);
  gUi.activeMenuPage = page;
  if (page == MenuPage::brightness) {
    refreshBrightnessTelemetry();
  }
}

void openMenu() {
  if (gUi.menuVisible) {
    return;
  }
  setFullScreenWarningVisible(false);
  refreshMenuControls();
  refreshBrightnessTelemetry();
  showMenuPage(MenuPage::home);
  lv_obj_clear_flag(gUi.menu, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(gUi.menu);
  gUi.menuVisible = true;
}

void gaugeLongPressEvent(lv_event_t* event) {
  if (lv_event_get_code(event) == LV_EVENT_LONG_PRESSED) {
    openMenu();
  }
}

void closeMenuEvent(lv_event_t*) {
  closeMenu(true);
}

void sectionBackEvent(lv_event_t*) {
  showMenuPage(MenuPage::home);
}

void brightnessPageEvent(lv_event_t*) {
  showMenuPage(MenuPage::brightness);
}

void dataPageEvent(lv_event_t*) {
  showMenuPage(MenuPage::data);
}

void warningsPageEvent(lv_event_t*) {
  showMenuPage(MenuPage::warnings);
}

void soundPageEvent(lv_event_t*) {
  showMenuPage(MenuPage::sound);
}

void unitsPageEvent(lv_event_t*) {
  showMenuPage(MenuPage::units);
}

void startupPageEvent(lv_event_t*) {
  showMenuPage(MenuPage::startup);
}

void systemPageEvent(lv_event_t*) {
  showMenuPage(MenuPage::system);
}

void sliderEvent(lv_event_t* event) {
  lv_obj_t* target = lv_event_get_target_obj(event);
  if (target == gUi.brightnessSlider) {
    gUi.settings.brightnessPercent = static_cast<std::uint8_t>(
        lv_slider_get_value(gUi.brightnessSlider));
    refreshBrightnessValueLabel();
    refreshMenuSummaries();
  } else if (target == gUi.automaticBrightnessRangeSlider) {
    gUi.settings.automaticBrightnessMinimumPercent =
        static_cast<std::uint8_t>(
            lv_slider_get_left_value(gUi.automaticBrightnessRangeSlider));
    gUi.settings.automaticBrightnessMaximumPercent =
        static_cast<std::uint8_t>(
            lv_slider_get_value(gUi.automaticBrightnessRangeSlider));
    refreshAutomaticBrightnessRangeLabel();
    refreshMenuSummaries();
  } else if (target == gUi.volumeSlider) {
    gUi.settings.warningVolumePercent = static_cast<std::uint8_t>(
        lv_slider_get_value(gUi.volumeSlider));
    refreshVolumeValueLabel();
    refreshMenuSummaries();
  } else if (target == gUi.pressureWarningSlider) {
    const std::int32_t value = lv_slider_get_value(gUi.pressureWarningSlider);
    gUi.settings.lowPressureWarningPsi = warningThresholdPsiFromDisplay(
        gUi.settings.pressureUnit == PressureUnit::bar ? value / 10.0 : value,
        gUi.settings.pressureUnit);
    refreshMenuControls();
  } else if (target == gUi.temperatureWarningSlider) {
    gUi.settings.highTemperatureWarningCelsius =
        temperatureWarningThresholdCelsiusFromDisplay(
            lv_slider_get_value(gUi.temperatureWarningSlider),
            gUi.settings.temperatureUnit);
    refreshMenuControls();
  } else if (target == gUi.startupLogoSlider) {
    gUi.settings.startupLogoSeconds = static_cast<std::uint8_t>(
        lv_slider_get_value(gUi.startupLogoSlider));
    refreshMenuControls();
  }
  queueSettingsApply();
}

void soundSwitchEvent(lv_event_t*) {
  gUi.settings.warningSoundEnabled =
      lv_obj_has_state(gUi.soundSwitch, LV_STATE_CHECKED);
  refreshMenuSummaries();
  queueSettingsApply();
}

void testSoundEvent(lv_event_t*) {
  gUi.pendingActions.testSound = true;
  gUi.actionsPending = true;
}

void unitEvent(lv_event_t* event) {
  gUi.settings.pressureUnit =
      lv_event_get_target_obj(event) == gUi.unitBarButton
          ? PressureUnit::bar
          : PressureUnit::psi;
  refreshMenuControls();
  queueSettingsApply();
}

void temperatureUnitEvent(lv_event_t* event) {
  gUi.settings.temperatureUnit =
      lv_event_get_target_obj(event) == gUi.unitFahrenheitButton
          ? TemperatureUnit::fahrenheit
          : TemperatureUnit::celsius;
  refreshMenuControls();
  queueSettingsApply();
}

void dataSourceEvent(lv_event_t* event) {
  gUi.settings.dataSource =
      lv_event_get_target_obj(event) == gUi.sensorsButton
          ? DataSource::sensors
          : DataSource::demo;
  refreshMenuControls();
  queueSettingsApply();
}

void brightnessModeEvent(lv_event_t* event) {
  gUi.settings.brightnessMode =
      lv_event_get_target_obj(event) == gUi.brightnessManualButton
          ? BrightnessMode::manual
          : BrightnessMode::automatic;
  refreshMenuControls();
  queueSettingsApply();
}

void warningModeEvent(lv_event_t* event) {
  lv_obj_t* target = lv_event_get_target_obj(event);
  if (target == gUi.warningScreenButton) {
    gUi.settings.warningVisualMode = WarningVisualMode::fullScreenBlink;
  } else if (target == gUi.warningFixedButton) {
    gUi.settings.warningVisualMode = WarningVisualMode::fixed;
  } else {
    gUi.settings.warningVisualMode = WarningVisualMode::elementsBlink;
  }
  refreshMenuControls();
  queueSettingsApply();
}

void resetRequestEvent(lv_event_t*) {
  lv_obj_clear_flag(gUi.resetConfirm, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(gUi.resetConfirm);
}

void resetCancelEvent(lv_event_t*) {
  lv_obj_add_flag(gUi.resetConfirm, LV_OBJ_FLAG_HIDDEN);
}

void resetConfirmEvent(lv_event_t*) {
  gUi.settings = sanitizeGaugeSettings(gUi.defaults);
  refreshMenuControls();
  queueSettingsApply();
  gUi.pendingActions.saveSettings = true;
  gUi.menuDirty = false;
  lv_obj_add_flag(gUi.resetConfirm, LV_OBJ_FLAG_HIDDEN);
}

lv_obj_t* createMenuButton(lv_obj_t* parent,
                           const char* text,
                           std::int32_t x,
                           std::int32_t y,
                           std::int32_t width,
                           std::int32_t height,
                           lv_event_cb_t callback) {
  lv_obj_t* button = lv_button_create(parent);
  lv_obj_remove_style_all(button);
  lv_obj_set_pos(button, x, y);
  lv_obj_set_size(button, width, height);
  lv_obj_set_style_radius(button, 12, 0);
  lv_obj_set_style_bg_color(button, color(kPanel), 0);
  lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(button, 1, 0);
  lv_obj_set_style_border_color(button, color(kLine), 0);
  lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, nullptr);
  createLabel(button,
              text,
              6,
              (height - 20) / 2,
              width - 12,
              20,
              &oil_font_ui_16,
              color(kPrimary),
              LV_TEXT_ALIGN_CENTER);
  return button;
}

lv_obj_t* createSectionButton(lv_obj_t* parent,
                              const char* title,
                              std::int32_t x,
                              std::int32_t y,
                              std::int32_t width,
                              std::int32_t height,
                              lv_event_cb_t callback,
                              lv_obj_t*& summaryLabel) {
  lv_obj_t* button = lv_button_create(parent);
  lv_obj_remove_style_all(button);
  lv_obj_set_pos(button, x, y);
  lv_obj_set_size(button, width, height);
  lv_obj_set_style_radius(button, 16, 0);
  lv_obj_set_style_bg_color(button, color(kPanel), 0);
  lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(button, 1, 0);
  lv_obj_set_style_border_color(button, color(kLine), 0);
  lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, nullptr);
  createLabel(button,
              title,
              14,
              15,
              width - 28,
              22,
              &oil_font_ui_16,
              color(kPrimary),
              LV_TEXT_ALIGN_LEFT);
  summaryLabel = createLabel(button,
                             "--",
                             14,
                             47,
                             width - 28,
                             18,
                             &oil_font_ui_12,
                             color(kSecondary),
                             LV_TEXT_ALIGN_LEFT);
  return button;
}

lv_obj_t* createSettingsPage(MenuPage page,
                             const char* title,
                             bool homePage) {
  lv_obj_t* pageObject =
      createSolid(gUi.menu, 0, 0, kCanvasWidth, kCanvasWidth, 0);
  lv_obj_set_style_bg_color(pageObject, color(kBlack), 0);
  gUi.menuPages[static_cast<std::size_t>(page)] = pageObject;
  createLabel(pageObject,
              title,
              28,
              22,
              280,
              34,
              &lv_font_montserrat_24,
              color(kPrimary),
              LV_TEXT_ALIGN_LEFT);
  createMenuButton(pageObject,
                   homePage ? "CERRAR" : "ATRÁS",
                   340,
                   12,
                   112,
                   48,
                   homePage ? closeMenuEvent : sectionBackEvent);
  if (!homePage) {
    lv_obj_add_flag(pageObject, LV_OBJ_FLAG_HIDDEN);
  }
  return pageObject;
}

void styleSlider(lv_obj_t* slider) {
  lv_obj_set_style_bg_color(slider, color(kLine), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(slider, color(kPrimary), LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider, color(kPrimary), LV_PART_KNOB);
}

lv_obj_t* createPressureIcon(lv_obj_t* parent) {
  lv_obj_t* icon = lv_image_create(parent);
  lv_obj_remove_style_all(icon);
  lv_obj_set_pos(icon, 28, 90);
  lv_image_set_src(icon, &oil_pressure_icon);
  lv_obj_set_style_image_recolor_opa(icon, LV_OPA_COVER, 0);
  lv_obj_clear_flag(icon, LV_OBJ_FLAG_SCROLLABLE);
  return icon;
}

lv_obj_t* createTemperatureIcon(lv_obj_t* parent) {
  lv_obj_t* icon = lv_image_create(parent);
  lv_obj_remove_style_all(icon);
  lv_obj_set_pos(icon, 28, 328);
  lv_image_set_src(icon, &oil_temperature_icon);
  lv_obj_set_style_image_recolor_opa(icon, LV_OPA_COVER, 0);
  lv_obj_clear_flag(icon, LV_OBJ_FLAG_SCROLLABLE);
  return icon;
}

void createSettingsMenu(lv_obj_t* screen) {
  gUi.menu = createSolid(screen, 0, 0, kCanvasWidth, kCanvasWidth, 0);
  lv_obj_set_style_bg_color(gUi.menu, color(kBlack), 0);

  lv_obj_t* home = createSettingsPage(MenuPage::home, "AJUSTES", true);
  createSectionButton(home,
                      "BRILLO",
                      28,
                      82,
                      206,
                      80,
                      brightnessPageEvent,
                      gUi.homeBrightnessSummary);
  createSectionButton(home,
                      "DATOS",
                      246,
                      82,
                      206,
                      80,
                      dataPageEvent,
                      gUi.homeDataSummary);
  createSectionButton(home,
                      "AVISOS",
                      28,
                      173,
                      206,
                      80,
                      warningsPageEvent,
                      gUi.homeWarningsSummary);
  createSectionButton(home,
                      "SONIDO",
                      246,
                      173,
                      206,
                      80,
                      soundPageEvent,
                      gUi.homeSoundSummary);
  createSectionButton(home,
                      "UNIDADES",
                      28,
                      264,
                      206,
                      80,
                      unitsPageEvent,
                      gUi.homeUnitsSummary);
  createSectionButton(home,
                      "ARRANQUE",
                      246,
                      264,
                      206,
                      80,
                      startupPageEvent,
                      gUi.homeStartupSummary);
  lv_obj_t* systemSummary = nullptr;
  createSectionButton(home,
                      "SISTEMA",
                      28,
                      356,
                      424,
                      92,
                      systemPageEvent,
                      systemSummary);
  lv_label_set_text(systemSummary, "RESTABLECER AJUSTES");

  lv_obj_t* brightness =
      createSettingsPage(MenuPage::brightness, "BRILLO", false);
  createLabel(brightness,
              "MODO DE BRILLO",
              28,
              82,
              220,
              24,
              &oil_font_ui_16,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  gUi.brightnessAutoButton = createMenuButton(
      brightness, "AUTO", 28, 112, 150, 48, brightnessModeEvent);
  gUi.brightnessManualButton = createMenuButton(
      brightness, "MANUAL", 194, 112, 150, 48, brightnessModeEvent);
  gUi.brightnessValue = createLabel(brightness,
                                    "RESPALDO MANUAL: 55%",
                                    28,
                                    178,
                                    360,
                                    24,
                                    &oil_font_ui_16,
                                    color(kPrimary),
                                    LV_TEXT_ALIGN_LEFT);
  gUi.brightnessSlider = lv_slider_create(brightness);
  lv_obj_set_pos(gUi.brightnessSlider, 28, 214);
  lv_obj_set_size(gUi.brightnessSlider, 424, 16);
  lv_slider_set_range(gUi.brightnessSlider, 5, 100);
  styleSlider(gUi.brightnessSlider);
  lv_obj_add_event_cb(
      gUi.brightnessSlider, sliderEvent, LV_EVENT_VALUE_CHANGED, nullptr);
  gUi.automaticBrightnessRangeValue = createLabel(brightness,
                                                   "LÍMITES AUTO: 20–100%",
                                                   28,
                                                   254,
                                                   424,
                                                   24,
                                                   &oil_font_ui_16,
                                                   color(kPrimary),
                                                   LV_TEXT_ALIGN_LEFT);
  gUi.automaticBrightnessRangeSlider = lv_slider_create(brightness);
  lv_obj_set_pos(gUi.automaticBrightnessRangeSlider, 28, 290);
  lv_obj_set_size(gUi.automaticBrightnessRangeSlider, 424, 16);
  lv_slider_set_mode(
      gUi.automaticBrightnessRangeSlider, LV_SLIDER_MODE_RANGE);
  lv_slider_set_range(gUi.automaticBrightnessRangeSlider, 5, 100);
  styleSlider(gUi.automaticBrightnessRangeSlider);
  lv_obj_add_event_cb(gUi.automaticBrightnessRangeSlider,
                      sliderEvent,
                      LV_EVENT_VALUE_CHANGED,
                      nullptr);
  gUi.ambientLuxLabel = createLabel(brightness,
                                     "LUZ HUB: --",
                                     28,
                                     326,
                                    424,
                                    20,
                                    &oil_font_ui_12,
                                    color(kPrimary),
                                    LV_TEXT_ALIGN_LEFT);
  gUi.ambientStateLabel = createLabel(brightness,
                                       "UART: ESPERANDO LUZ",
                                       28,
                                       350,
                                      424,
                                      20,
                                      &oil_font_ui_12,
                                      color(kSecondary),
                                      LV_TEXT_ALIGN_LEFT);
  gUi.automaticBrightnessLabel = createLabel(brightness,
                                               "AUTO ESPERANDO · APLICADO: 55%",
                                               28,
                                               374,
                                              424,
                                              20,
                                              &oil_font_ui_12,
                                              color(kSecondary),
                                              LV_TEXT_ALIGN_LEFT);

  lv_obj_t* data = createSettingsPage(MenuPage::data, "DATOS", false);
  createLabel(data,
              "FUENTE DE DATOS",
              28,
              92,
              260,
              24,
              &oil_font_ui_16,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  gUi.demoButton =
      createMenuButton(data, "DEMO", 28, 126, 150, 50, dataSourceEvent);
  gUi.sensorsButton = createMenuButton(
      data, "SENSORES", 194, 126, 150, 50, dataSourceEvent);
  createLabel(data,
              "DEMO GENERA LA SECUENCIA DE PRUEBA",
              28,
              210,
              424,
              20,
              &oil_font_ui_12,
              color(kPrimary),
              LV_TEXT_ALIGN_LEFT);
  createLabel(data,
              "TEMPERATURA A1: CALIBRACIÓN PROVISIONAL",
              28,
              246,
              424,
              20,
              &oil_font_ui_12,
              color(kWarningRed),
              LV_TEXT_ALIGN_LEFT);
  createLabel(data,
              "PRESIÓN: CALIBRACIÓN PENDIENTE",
              28,
              276,
              424,
              20,
              &oil_font_ui_12,
              color(kWarningRed),
              LV_TEXT_ALIGN_LEFT);

  lv_obj_t* warnings =
      createSettingsPage(MenuPage::warnings, "AVISOS", false);
  createLabel(warnings,
              "PRESIÓN BAJA",
              28,
              82,
              240,
              24,
              &oil_font_ui_16,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  gUi.pressureWarningValue = createLabel(warnings,
                                         "UMBRAL: 10 PSI",
                                         28,
                                         112,
                                         260,
                                         24,
                                         &oil_font_ui_16,
                                         color(kPrimary),
                                         LV_TEXT_ALIGN_LEFT);
  gUi.pressureWarningSlider = lv_slider_create(warnings);
  lv_obj_set_pos(gUi.pressureWarningSlider, 28, 148);
  lv_obj_set_size(gUi.pressureWarningSlider, 424, 16);
  styleSlider(gUi.pressureWarningSlider);
  lv_obj_add_event_cb(gUi.pressureWarningSlider,
                      sliderEvent,
                      LV_EVENT_VALUE_CHANGED,
                      nullptr);
  createLabel(warnings,
              "TEMPERATURA ALTA",
              28,
              190,
              260,
              24,
              &oil_font_ui_16,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  gUi.temperatureWarningValue = createLabel(warnings,
                                            "UMBRAL: 120 °C",
                                            28,
                                            220,
                                            260,
                                            24,
                                            &oil_font_ui_16,
                                            color(kPrimary),
                                            LV_TEXT_ALIGN_LEFT);
  gUi.temperatureWarningSlider = lv_slider_create(warnings);
  lv_obj_set_pos(gUi.temperatureWarningSlider, 28, 256);
  lv_obj_set_size(gUi.temperatureWarningSlider, 424, 16);
  lv_slider_set_range(gUi.temperatureWarningSlider, 110, 140);
  styleSlider(gUi.temperatureWarningSlider);
  lv_obj_add_event_cb(gUi.temperatureWarningSlider,
                      sliderEvent,
                      LV_EVENT_VALUE_CHANGED,
                      nullptr);
  createLabel(warnings,
              "AVISO VISUAL",
              28,
              298,
              240,
              24,
              &oil_font_ui_16,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  gUi.warningElementsButton = createMenuButton(
      warnings, "ELEMENTOS", 28, 330, 136, 50, warningModeEvent);
  gUi.warningScreenButton = createMenuButton(
      warnings, "PANTALLA", 172, 330, 136, 50, warningModeEvent);
  gUi.warningFixedButton = createMenuButton(
      warnings, "FIJO", 316, 330, 136, 50, warningModeEvent);
  gUi.warningModeDescription = createLabel(warnings,
                                           "ELEMENTOS · 2 HZ",
                                           28,
                                           398,
                                           424,
                                           20,
                                           &oil_font_ui_12,
                                           color(kSecondary),
                                           LV_TEXT_ALIGN_LEFT);

  lv_obj_t* sound = createSettingsPage(MenuPage::sound, "SONIDO", false);
  createLabel(sound,
              "SONIDO DE AVISO",
              28,
              96,
              240,
              24,
              &oil_font_ui_16,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  createLabel(sound,
              "ACTIVADO",
              28,
              132,
              190,
              24,
              &oil_font_ui_16,
              color(kPrimary),
              LV_TEXT_ALIGN_LEFT);
  gUi.soundSwitch = lv_switch_create(sound);
  lv_obj_set_pos(gUi.soundSwitch, 370, 123);
  lv_obj_set_size(gUi.soundSwitch, 82, 42);
  lv_obj_add_event_cb(
      gUi.soundSwitch, soundSwitchEvent, LV_EVENT_VALUE_CHANGED, nullptr);
  gUi.volumeValue = createLabel(sound,
                                "VOLUMEN: 35%",
                                28,
                                190,
                                220,
                                24,
                                &oil_font_ui_16,
                                color(kPrimary),
                                LV_TEXT_ALIGN_LEFT);
  gUi.volumeSlider = lv_slider_create(sound);
  lv_obj_set_pos(gUi.volumeSlider, 28, 228);
  lv_obj_set_size(gUi.volumeSlider, 424, 16);
  lv_slider_set_range(gUi.volumeSlider, 5, 100);
  styleSlider(gUi.volumeSlider);
  lv_obj_add_event_cb(
      gUi.volumeSlider, sliderEvent, LV_EVENT_VALUE_CHANGED, nullptr);
  createMenuButton(sound, "PROBAR SONIDO", 28, 286, 200, 52, testSoundEvent);

  lv_obj_t* units = createSettingsPage(MenuPage::units, "UNIDADES", false);
  createLabel(units,
              "PRESIÓN",
              28,
              96,
              180,
              24,
              &oil_font_ui_16,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  gUi.unitPsiButton =
      createMenuButton(units, "PSI", 28, 130, 150, 50, unitEvent);
  gUi.unitBarButton =
      createMenuButton(units, "BAR", 194, 130, 150, 50, unitEvent);
  createLabel(units,
              "TEMPERATURA",
              28,
              218,
              220,
              24,
              &oil_font_ui_16,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  gUi.unitCelsiusButton = createMenuButton(
      units, "°C", 28, 252, 150, 50, temperatureUnitEvent);
  gUi.unitFahrenheitButton = createMenuButton(
      units, "°F", 194, 252, 150, 50, temperatureUnitEvent);

  lv_obj_t* startup =
      createSettingsPage(MenuPage::startup, "ARRANQUE", false);
  createLabel(startup,
              "LOGOTIPO HONDA / CIVIC",
              28,
              96,
              300,
              24,
              &oil_font_ui_16,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  gUi.startupLogoValue = createLabel(startup,
                                     "DURACIÓN: 1 S",
                                     28,
                                     134,
                                     300,
                                     24,
                                     &oil_font_ui_16,
                                     color(kPrimary),
                                     LV_TEXT_ALIGN_LEFT);
  gUi.startupLogoSlider = lv_slider_create(startup);
  lv_obj_set_pos(gUi.startupLogoSlider, 28, 172);
  lv_obj_set_size(gUi.startupLogoSlider, 424, 16);
  lv_slider_set_range(gUi.startupLogoSlider, 0, 10);
  styleSlider(gUi.startupLogoSlider);
  lv_obj_add_event_cb(gUi.startupLogoSlider,
                      sliderEvent,
                      LV_EVENT_VALUE_CHANGED,
                      nullptr);
  createLabel(startup,
              "0 SEGUNDOS DESACTIVA EL LOGOTIPO",
              28,
              216,
              424,
              20,
              &oil_font_ui_12,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);

  lv_obj_t* system =
      createSettingsPage(MenuPage::system, "SISTEMA", false);
  createLabel(system,
              "LOS CAMBIOS SE GUARDAN AL CERRAR",
              28,
              100,
              424,
              24,
              &oil_font_ui_16,
              color(kPrimary),
              LV_TEXT_ALIGN_LEFT);
  createLabel(system,
              "RECUPERA TODOS LOS VALORES PREDETERMINADOS",
              28,
              146,
              424,
              20,
              &oil_font_ui_12,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  createMenuButton(
      system, "RESTABLECER", 28, 206, 190, 52, resetRequestEvent);

  gUi.resetConfirm = createSolid(screen, 30, 125, 420, 230, 18);
  lv_obj_set_style_bg_color(gUi.resetConfirm, color(kPanel), 0);
  lv_obj_set_style_border_width(gUi.resetConfirm, 2, 0);
  lv_obj_set_style_border_color(gUi.resetConfirm, color(kPrimary), 0);
  createLabel(gUi.resetConfirm,
              "¿RESTABLECER AJUSTES?",
              24,
              32,
              372,
              30,
              &lv_font_montserrat_24,
              color(kPrimary),
              LV_TEXT_ALIGN_CENTER);
  createLabel(gUi.resetConfirm,
              "VOLVERÁN LOS VALORES SEGUROS",
              24,
              82,
              372,
              24,
              &oil_font_ui_16,
              color(kSecondary),
              LV_TEXT_ALIGN_CENTER);
  createMenuButton(
      gUi.resetConfirm, "CANCELAR", 24, 148, 174, 54, resetCancelEvent);
  createMenuButton(
      gUi.resetConfirm, "RESTABLECER", 222, 148, 174, 54, resetConfirmEvent);
  lv_obj_add_flag(gUi.resetConfirm, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(gUi.menu, LV_OBJ_FLAG_HIDDEN);
}

void createFullScreenWarning(lv_obj_t* screen) {
  gUi.fullScreenWarning =
      createSolid(screen, 0, 0, kCanvasWidth, kCanvasWidth, 0);
  lv_obj_set_style_bg_color(gUi.fullScreenWarning, color(kWarningRed), 0);
  gUi.fullScreenPressureValue = createLabel(gUi.fullScreenWarning,
                                             "61",
                                             120,
                                             75,
                                             240,
                                             106,
                                             &oil_font_value_96,
                                             color(kPrimary),
                                             LV_TEXT_ALIGN_CENTER);
  lv_obj_set_style_text_letter_space(gUi.fullScreenPressureValue, -5, 0);
  createLabel(gUi.fullScreenWarning,
              "PELIGRO",
              38,
              210,
              404,
              48,
              &oil_font_warning_36,
              color(kPrimary),
              LV_TEXT_ALIGN_CENTER);
  createLabel(gUi.fullScreenWarning,
              "PRESIÓN MUY BAJA",
              38,
              266,
              404,
              48,
              &oil_font_warning_36,
              color(kPrimary),
              LV_TEXT_ALIGN_CENTER);
  lv_obj_add_flag(gUi.fullScreenWarning, LV_OBJ_FLAG_HIDDEN);
}

void createBootSplash(lv_obj_t* screen) {
  gUi.bootSplash = createSolid(screen, 0, 0, kCanvasWidth, kCanvasWidth, 0);
  lv_obj_set_style_bg_color(gUi.bootSplash, color(kBlack), 0);
  lv_obj_t* logo = lv_image_create(gUi.bootSplash);
  lv_obj_remove_style_all(logo);
  lv_image_set_src(logo, &startup_honda_logo);
  lv_obj_align(logo, LV_ALIGN_TOP_MID, 0, 92);
  lv_obj_clear_flag(logo, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_t* civic = lv_image_create(gUi.bootSplash);
  lv_obj_remove_style_all(civic);
  lv_image_set_src(civic, &startup_civic_logo);
  lv_obj_align(civic, LV_ALIGN_TOP_MID, 0, 319);
  lv_obj_clear_flag(civic, LV_OBJ_FLAG_SCROLLABLE);
  if (gUi.settings.startupLogoSeconds == 0) {
    lv_obj_add_flag(gUi.bootSplash, LV_OBJ_FLAG_HIDDEN);
  } else {
    gUi.bootSplashVisible = true;
  }
}

BarWidgets createBar(lv_obj_t* parent,
                     std::int32_t y,
                     const double* ticks,
                     std::size_t tickCount,
                     std::size_t warningTickIndex) {
  BarWidgets widgets;
  widgets.track = createSolid(
      parent, kContentX, y, kContentWidth, kBarHeight, LV_RADIUS_CIRCLE);
  lv_obj_set_style_bg_color(widgets.track, color(kLine), 0);

  for (std::size_t index = 0; index < tickCount; ++index) {
    const std::int32_t tickX = kContentX + static_cast<std::int32_t>(
        std::lround(ticks[index] * static_cast<double>(kContentWidth)));
    lv_obj_t* tick = createSolid(parent, tickX, y, 1, kBarHeight, 0);
    if (index == warningTickIndex) {
      widgets.firstTick = tick;
    }
    lv_obj_set_style_bg_color(tick, color(kPrimary), 0);
    lv_obj_set_style_bg_opa(tick, LV_OPA_30, 0);
  }
  widgets.fill = createSolid(
      parent, kContentX, y, 1, kBarHeight, LV_RADIUS_CIRCLE);
  return widgets;
}

void updateBar(BarWidgets& bar,
               double fraction,
               const RgbColor& fillColor,
               lv_opa_t opacity) {
  const double bounded = clamp(fraction, 0.0, 1.0);
  const double exactWidth = bounded * static_cast<double>(kContentWidth);
  const std::int32_t width = static_cast<std::int32_t>(std::lround(exactWidth));
  if (!bar.colorSet || !sameColor(bar.fillColor, fillColor)) {
    lv_obj_set_style_bg_color(bar.fill, color(fillColor), 0);
    bar.fillColor = fillColor;
    bar.colorSet = true;
  }
  if (bar.opacity != opacity) {
    lv_obj_set_style_opa(bar.fill, opacity, 0);
    bar.opacity = opacity;
  }
  if (width <= 0) {
    if (!bar.hidden) {
      lv_obj_add_flag(bar.fill, LV_OBJ_FLAG_HIDDEN);
      bar.hidden = true;
    }
  } else {
    if (bar.hidden) {
      lv_obj_clear_flag(bar.fill, LV_OBJ_FLAG_HIDDEN);
      bar.hidden = false;
    }
    if (bar.width != width) {
      lv_obj_set_width(bar.fill, width);
      bar.width = width;
    }
  }

}

const char* pressureLabel(PressureState state) {
  switch (state) {
    case PressureState::fault:
      return "FALLO SENSOR";
    case PressureState::engineUnknown:
      return "RPM SIN DATOS";
    case PressureState::engineStopped:
      return "MOTOR PARADO";
    case PressureState::warning:
      return "WARNING";
    case PressureState::low:
      return "PRESIÓN BAJA";
    case PressureState::ok:
      return "OK";
    case PressureState::high:
      return "PRESIÓN ALTA";
  }
  return "FALLO";
}

const char* temperatureLabel(TemperatureState state) {
  switch (state) {
    case TemperatureState::fault:
      return "FALLO SENSOR";
    case TemperatureState::belowRange:
    case TemperatureState::cold:
      return "FRÍO";
    case TemperatureState::warming:
      return "CALENTANDO";
    case TemperatureState::optimal:
      return "ÓPTIMO";
    case TemperatureState::veryHot:
      return "MUY CALIENTE";
    case TemperatureState::warning:
      return "WARNING";
  }
  return "FALLO";
}

}  // namespace

void createOilGaugeUi(lv_obj_t* screen,
                      const GaugeSettings& settings,
                      const GaugeSettings& defaults) {
  gUi = {};
  gUi.screen = screen;
  gUi.settings = sanitizeGaugeSettings(settings);
  gUi.defaults = sanitizeGaugeSettings(defaults);

  lv_obj_remove_style_all(screen);
  lv_obj_set_size(screen, kCanvasWidth, kCanvasWidth);
  lv_obj_set_style_bg_color(screen, color(kBlack), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

  gUi.gaugeRoot = createSolid(screen, 0, 0, kCanvasWidth, kCanvasWidth, 0);
  lv_obj_set_style_bg_color(gUi.gaugeRoot, color(kBlack), 0);
  lv_obj_add_event_cb(
      gUi.gaugeRoot, gaugeLongPressEvent, LV_EVENT_LONG_PRESSED, nullptr);

  lv_obj_t* divider =
      createSolid(gUi.gaugeRoot, 0, kHalfHeight - 1, 480, 1, 0);
  lv_obj_set_style_bg_color(divider, color(kLine), 0);

  gUi.sourceBadge = createLabel(
      gUi.gaugeRoot,
      gUi.settings.dataSource == DataSource::demo ? "DEMO" : "SENSOR A1",
      180,
      7,
      120,
      14,
      &oil_font_ui_12,
      color(kSecondary),
      LV_TEXT_ALIGN_CENTER);

  createLabel(gUi.gaugeRoot,
              "PRESIÓN ACEITE",
              kContentX,
              34,
              190,
              20,
              &oil_font_ui_16,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  gUi.pressureState = createLabel(gUi.gaugeRoot,
                                  "OK",
                                  230,
                                  28,
                                  212,
                                  32,
                                  &oil_font_ui_24,
                                  color(kSecondary),
                                  LV_TEXT_ALIGN_RIGHT);
  gUi.pressureIcon = createPressureIcon(gUi.gaugeRoot);
  gUi.pressureValue = createLabel(gUi.gaugeRoot,
                                  "61",
                                  120,
                                  79,
                                  240,
                                  106,
                                  &oil_font_value_96,
                                  color(kPrimary),
                                  LV_TEXT_ALIGN_CENTER);
  lv_obj_set_style_text_letter_space(gUi.pressureValue, -5, 0);
  gUi.pressureUnit = createLabel(gUi.gaugeRoot,
                                 "PSI",
                                 340,
                                 123,
                                 70,
                                 32,
                                 &lv_font_montserrat_28,
                                 color(kSecondary),
                                 LV_TEXT_ALIGN_LEFT);

  static constexpr double kPressureTicks[] = {0.094, 0.176, 0.941};
  gUi.pressureBar =
      createBar(gUi.gaugeRoot, 188, kPressureTicks, std::size(kPressureTicks),
                0);

  createLabel(gUi.gaugeRoot,
              "TEMPERATURA ACEITE",
              kContentX,
              274,
              230,
              20,
              &oil_font_ui_16,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  gUi.temperatureState = createLabel(gUi.gaugeRoot,
                                     "ÓPTIMO",
                                     230,
                                     268,
                                     212,
                                     32,
                                     &oil_font_ui_24,
                                     color(kSecondary),
                                     LV_TEXT_ALIGN_RIGHT);
  gUi.temperatureIcon = createTemperatureIcon(gUi.gaugeRoot);
  gUi.temperatureValue = createLabel(gUi.gaugeRoot,
                                     "92",
                                     120,
                                     319,
                                     240,
                                     106,
                                     &oil_font_value_96,
                                     color(kPrimary),
                                     LV_TEXT_ALIGN_CENTER);
  lv_obj_set_style_text_letter_space(gUi.temperatureValue, -5, 0);
  gUi.temperatureUnit = createLabel(gUi.gaugeRoot,
                                    "°C",
                                    340,
                                    363,
                                    70,
                                    32,
                                    &lv_font_montserrat_28,
                                    color(kSecondary),
                                    LV_TEXT_ALIGN_LEFT);

  static constexpr double kTemperatureTicks[] = {0.111, 0.289, 0.556, 0.778};
  gUi.temperatureBar = createBar(
      gUi.gaugeRoot, 428, kTemperatureTicks, std::size(kTemperatureTicks), 3);

  createSettingsMenu(screen);
  createFullScreenWarning(screen);
  createBootSplash(screen);
  refreshMenuControls();
  refreshBrightnessTelemetry();
  gUi.created = true;
}

void updateOilGaugeUi(const ConvertedValue& pressure,
                      const ConvertedValue& temperature,
                      const EngineState& engine,
                      bool elementsBlinkPhaseOn,
                      bool fullScreenBlinkPhaseOn,
                      const GaugeSettings& settings) {
  if (!gUi.created) {
    return;
  }

  gUi.settings = sanitizeGaugeSettings(settings);
  const bool reducedMotion =
      gUi.settings.warningVisualMode == WarningVisualMode::fixed;
  const DisplayState state = evaluateDisplayState(
      pressure,
      temperature,
      engine,
      elementsBlinkPhaseOn,
      reducedMotion,
      gUi.settings.lowPressureWarningPsi,
      gUi.settings.highTemperatureWarningCelsius);
  const bool warning = state.pressure == PressureState::warning;
  gUi.warningActive = warning;
  if (gUi.menuVisible) {
    return;
  }
  setLabelTextIfChanged(
      gUi.sourceBadge,
      gUi.sourceBadgeText,
      gUi.settings.dataSource == DataSource::demo ? "DEMO" : "SENSOR A1");
  const bool sensorSource = gUi.settings.dataSource == DataSource::sensors;
  const bool pressurePending = sensorSource && !pressure.valid();
  const bool temperaturePending = sensorSource && !temperature.valid();
  const RgbColor pressureColorValue =
      pressurePending ? RgbColor{154, 164, 175} : state.pressureColor;
  const RgbColor temperatureColorValue =
      temperaturePending ? RgbColor{154, 164, 175} : state.temperatureColor;
  char pressureText[8];
  if (pressure.valid()) {
    const double displayed =
        pressureForDisplay(pressure.value, gUi.settings.pressureUnit);
    if (gUi.settings.pressureUnit == PressureUnit::bar) {
      std::snprintf(pressureText, sizeof(pressureText), "%.1f", displayed);
    } else {
      std::snprintf(pressureText,
                    sizeof(pressureText),
                    "%d",
                    static_cast<int>(std::lround(displayed)));
    }
  } else {
    std::snprintf(pressureText, sizeof(pressureText), "--");
  }
  setLabelTextIfChanged(
      gUi.pressureValue, gUi.pressureValueText, pressureText);
  setLabelTextIfChanged(gUi.fullScreenPressureValue,
                        gUi.fullScreenPressureValueText,
                        pressureText);
  const WarningPresentation presentation = evaluateWarningPresentation(
      gUi.settings.warningVisualMode,
      warning,
      elementsBlinkPhaseOn,
      fullScreenBlinkPhaseOn);
  setFullScreenWarningVisible(presentation.fullScreenRedVisible);
  if (presentation.fullScreenRedVisible) {
    return;
  }
  if (!gUi.unitRendered || gUi.renderedUnit != gUi.settings.pressureUnit) {
    const bool bar = gUi.settings.pressureUnit == PressureUnit::bar;
    lv_label_set_text(gUi.pressureUnit, bar ? "BAR" : "PSI");
    gUi.renderedUnit = gUi.settings.pressureUnit;
    gUi.unitRendered = true;
  }
  if (!gUi.temperatureUnitRendered ||
      gUi.renderedTemperatureUnit != gUi.settings.temperatureUnit) {
    const bool fahrenheit =
        gUi.settings.temperatureUnit == TemperatureUnit::fahrenheit;
    lv_label_set_text(gUi.temperatureUnit, fahrenheit ? "°F" : "°C");
    gUi.renderedTemperatureUnit = gUi.settings.temperatureUnit;
    gUi.temperatureUnitRendered = true;
  }
  setLabelTextIfChanged(gUi.pressureState,
                        gUi.pressureStateText,
                        pressurePending ? "SIN DATOS"
                                        : pressureLabel(state.pressure));
  if (!gUi.pressureColorSet ||
      !sameColor(gUi.pressureColor, pressureColorValue)) {
    const lv_color_t pressureColor = color(pressureColorValue);
    lv_obj_set_style_text_color(gUi.pressureState, pressureColor, 0);
    gUi.pressureColor = pressureColorValue;
    gUi.pressureColorSet = true;
  }
  const RgbColor pressureIconColorValue =
      pressurePending ? RgbColor{154, 164, 175} : state.pressureIconColor;
  if (!gUi.pressureIconColorSet ||
      !sameColor(gUi.pressureIconColor, pressureIconColorValue)) {
    setIconColor(gUi.pressureIcon, color(pressureIconColorValue));
    gUi.pressureIconColor = pressureIconColorValue;
    gUi.pressureIconColorSet = true;
  }

  const lv_opa_t attentionOpacity =
      presentation.attentionVisible ? LV_OPA_COVER : LV_OPA_TRANSP;
  if (gUi.pressureAttentionOpacity != attentionOpacity) {
    lv_obj_set_style_opa(gUi.pressureIcon, attentionOpacity, 0);
    lv_obj_set_style_text_opa(gUi.pressureState, attentionOpacity, 0);
    gUi.pressureAttentionOpacity = attentionOpacity;
  }
  updateBar(gUi.pressureBar,
            state.pressureBarFraction,
            pressureColorValue,
            attentionOpacity);

  char temperatureText[8];
  if (!temperature.valid()) {
    std::snprintf(temperatureText, sizeof(temperatureText), "--");
  } else if (state.showTemperatureBelowRange && !sensorSource) {
    std::snprintf(temperatureText,
                  sizeof(temperatureText),
                  "%s",
                  gUi.settings.temperatureUnit == TemperatureUnit::fahrenheit
                      ? "<122"
                      : "<50");
  } else {
    const double displayedTemperature = temperatureForDisplay(
        temperature.value, gUi.settings.temperatureUnit);
    std::snprintf(temperatureText,
                  sizeof(temperatureText),
                  "%d",
                  static_cast<int>(std::lround(displayedTemperature)));
  }
  setLabelTextIfChanged(
      gUi.temperatureValue, gUi.temperatureValueText, temperatureText);
  setLabelTextIfChanged(gUi.temperatureState,
                        gUi.temperatureStateText,
                        temperaturePending
                            ? "SIN DATOS"
                            : temperatureLabel(state.temperature));
  if (!gUi.temperatureColorSet ||
      !sameColor(gUi.temperatureColor, temperatureColorValue)) {
    const lv_color_t temperatureLvColor = color(temperatureColorValue);
    lv_obj_set_style_text_color(
        gUi.temperatureState, temperatureLvColor, 0);
    gUi.temperatureColor = temperatureColorValue;
    gUi.temperatureColorSet = true;
  }
  const RgbColor temperatureIconColorValue =
      temperaturePending ? RgbColor{154, 164, 175}
                         : state.temperatureIconColor;
  if (!gUi.temperatureIconColorSet ||
      !sameColor(gUi.temperatureIconColor, temperatureIconColorValue)) {
    setIconColor(gUi.temperatureIcon, color(temperatureIconColorValue));
    gUi.temperatureIconColor = temperatureIconColorValue;
    gUi.temperatureIconColorSet = true;
  }
  const bool temperatureAttentionHidden =
      !temperaturePending && !state.temperatureAttentionVisible;
  if (gUi.temperatureAttentionHidden != temperatureAttentionHidden) {
    if (temperatureAttentionHidden) {
      lv_obj_add_flag(gUi.temperatureIcon, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(gUi.temperatureState, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_clear_flag(gUi.temperatureIcon, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(gUi.temperatureState, LV_OBJ_FLAG_HIDDEN);
    }
    gUi.temperatureAttentionHidden = temperatureAttentionHidden;
  }
  updateBar(gUi.temperatureBar,
            state.temperatureBarFraction,
            temperatureColorValue,
            LV_OPA_COVER);
}

void setOilGaugeBootSplashVisible(bool visible) {
  if (!gUi.created || gUi.bootSplash == nullptr) {
    return;
  }
  if (visible == gUi.bootSplashVisible) {
    return;
  }
  if (visible) {
    lv_obj_clear_flag(gUi.bootSplash, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(gUi.bootSplash);
  } else {
    lv_obj_add_flag(gUi.bootSplash, LV_OBJ_FLAG_HIDDEN);
  }
  gUi.bootSplashVisible = visible;
}

void updateOilGaugeBrightnessStatus(const OilGaugeBrightnessStatus& status) {
  if (sameBrightnessStatus(gUi.brightnessStatus, status)) {
    return;
  }
  gUi.brightnessStatus = status;
  if (gUi.created && gUi.menuVisible &&
      gUi.activeMenuPage == MenuPage::brightness) {
    refreshBrightnessTelemetry();
  }
}

bool oilGaugeFullScreenWarningVisible() {
  return gUi.fullScreenWarningVisible;
}

bool takeOilGaugeUiActions(OilGaugeUiActions& actions) {
  if (!gUi.actionsPending) {
    return false;
  }
  actions = gUi.pendingActions;
  gUi.pendingActions = {};
  gUi.pendingActions.settings = gUi.settings;
  gUi.actionsPending = false;
  return true;
}

}  // namespace oilgauge
