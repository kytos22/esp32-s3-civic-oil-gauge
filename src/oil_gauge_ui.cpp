#include "oil_gauge_ui.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <iterator>

#include "oil_gauge_fonts.h"
#include "sdkconfig.h"

namespace oilgauge {

namespace {

constexpr std::int32_t kCanvasWidth = 480;
constexpr std::int32_t kHalfHeight = 240;
constexpr std::int32_t kContentX = 38;
constexpr std::int32_t kContentWidth = 404;
constexpr std::int32_t kBarHeight = 9;

constexpr std::uint32_t kBlack = 0x000000;
constexpr std::uint32_t kPrimary = 0xF7F9FB;
constexpr std::uint32_t kSecondary = 0x9AA4AF;
constexpr std::uint32_t kLine = 0x262728;

struct BarWidgets {
  lv_obj_t* track = nullptr;
  lv_obj_t* fill = nullptr;
  lv_obj_t* edge = nullptr;
  std::int32_t width = -1;
  std::int32_t edgeX = -1;
  RgbColor fillColor{};
  lv_opa_t opacity = LV_OPA_TRANSP;
  lv_opa_t edgeOpacity = LV_OPA_TRANSP;
  bool colorSet = false;
  bool hidden = false;
  bool edgeHidden = true;
};

struct UiWidgets {
  lv_obj_t* screen = nullptr;
  lv_obj_t* pressureState = nullptr;
  lv_obj_t* pressureValue = nullptr;
  lv_obj_t* pressureIcon = nullptr;
  BarWidgets pressureBar{};
  lv_obj_t* temperatureState = nullptr;
  lv_obj_t* temperatureValue = nullptr;
  lv_obj_t* temperatureIcon = nullptr;
  BarWidgets temperatureBar{};
  char pressureValueText[8]{};
  char pressureStateText[24]{};
  char temperatureValueText[8]{};
  char temperatureStateText[24]{};
  RgbColor pressureColor{};
  RgbColor temperatureColor{};
  lv_opa_t pressureAttentionOpacity = LV_OPA_TRANSP;
  bool pressureColorSet = false;
  bool temperatureColorSet = false;
  bool created = false;
};

UiWidgets gUi;

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

lv_obj_t* createLine(lv_obj_t* parent,
                     const lv_point_precise_t* points,
                     std::uint16_t pointCount,
                     std::int32_t width) {
  lv_obj_t* line = lv_line_create(parent);
  lv_obj_remove_style_all(line);
  lv_line_set_points(line, points, pointCount);
  lv_obj_set_style_line_width(line, width, 0);
  lv_obj_set_style_line_rounded(line, true, 0);
  return line;
}

void setIconColor(lv_obj_t* icon, lv_color_t iconColor) {
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

lv_obj_t* createPressureIcon(lv_obj_t* parent) {
  lv_obj_t* icon = lv_obj_create(parent);
  lv_obj_remove_style_all(icon);
  lv_obj_set_pos(icon, 38, 95);
  lv_obj_set_size(icon, 84, 54);
  lv_obj_clear_flag(icon, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(icon, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

  static constexpr lv_point_precise_t kPickup[] = {
      {9, 24}, {16, 9}, {32, 17}};
  static constexpr lv_point_precise_t kCanBody[] = {
      {28, 17}, {32, 39}, {56, 39}, {71, 18}};
  static constexpr lv_point_precise_t kCanTop[] = {
      {29, 17}, {48, 17}, {56, 23}, {71, 18}, {79, 24}};
  static constexpr lv_point_precise_t kCapStem[] = {{42, 16}, {42, 8}};
  static constexpr lv_point_precise_t kCapTop[] = {{37, 7}, {47, 7}};

  createLine(icon, kPickup, 3, 7);
  createLine(icon, kCanBody, 4, 7);
  createLine(icon, kCanTop, 5, 7);
  createLine(icon, kCapStem, 2, 7);
  createLine(icon, kCapTop, 2, 7);

  lv_obj_t* dropletPoint = createSolid(icon, 72, 29, 9, 9, 1);
  lv_obj_set_style_transform_pivot_x(dropletPoint, 4, 0);
  lv_obj_set_style_transform_pivot_y(dropletPoint, 4, 0);
  lv_obj_set_style_transform_rotation(dropletPoint, 450, 0);
  createSolid(icon, 69, 34, 15, 15, LV_RADIUS_CIRCLE);
  return icon;
}

lv_obj_t* createTemperatureIcon(lv_obj_t* parent) {
  lv_obj_t* icon = lv_obj_create(parent);
  lv_obj_remove_style_all(icon);
  lv_obj_set_pos(icon, 40, 333);
  lv_obj_set_size(icon, 78, 66);
  lv_obj_clear_flag(icon, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(icon, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

  createSolid(icon, 31, 3, 10, 39, 5);
  createSolid(icon, 25, 32, 22, 22, LV_RADIUS_CIRCLE);
  createSolid(icon, 42, 9, 18, 6, 1);
  createSolid(icon, 42, 22, 18, 6, 1);
  createSolid(icon, 42, 34, 18, 6, 1);

  static constexpr lv_point_precise_t kWaveLeft[] = {
      {4, 47}, {9, 43}, {14, 43}, {19, 47}, {24, 47}};
  static constexpr lv_point_precise_t kWaveRight[] = {
      {48, 47}, {53, 43}, {58, 43}, {63, 47}, {68, 47}};
  static constexpr lv_point_precise_t kWaveBottom[] = {
      {7, 59},  {12, 55}, {17, 55}, {22, 59}, {27, 59}, {33, 55},
      {38, 55}, {43, 59}, {48, 59}, {54, 55}, {59, 55}, {66, 59}};

  createLine(icon, kWaveLeft, 5, 6);
  createLine(icon, kWaveRight, 5, 6);
  createLine(icon, kWaveBottom, 12, 6);
  return icon;
}

BarWidgets createBar(lv_obj_t* parent,
                     std::int32_t y,
                     const double* ticks,
                     std::size_t tickCount) {
  BarWidgets widgets;
  widgets.track = createSolid(
      parent, kContentX, y, kContentWidth, kBarHeight, LV_RADIUS_CIRCLE);
  lv_obj_set_style_bg_color(widgets.track, color(kLine), 0);

  widgets.fill = createSolid(
      parent, kContentX, y, 1, kBarHeight, LV_RADIUS_CIRCLE);
  widgets.edge = createSolid(parent, kContentX, y, 1, kBarHeight, 0);
  lv_obj_add_flag(widgets.edge, LV_OBJ_FLAG_HIDDEN);

  for (std::size_t index = 0; index < tickCount; ++index) {
    const std::int32_t tickX = kContentX + static_cast<std::int32_t>(
        std::lround(ticks[index] * static_cast<double>(kContentWidth)));
    lv_obj_t* tick = createSolid(parent, tickX, y - 2, 1, 13, 0);
    lv_obj_set_style_bg_color(tick, color(kPrimary), 0);
    lv_obj_set_style_bg_opa(tick, LV_OPA_30, 0);
  }
  return widgets;
}

void updateBar(BarWidgets& bar,
               double fraction,
               const RgbColor& fillColor,
               lv_opa_t opacity) {
  const double bounded = clamp(fraction, 0.0, 1.0);
  const double exactWidth = bounded * static_cast<double>(kContentWidth);
  std::int32_t width = static_cast<std::int32_t>(std::floor(exactWidth));
  double edgeFraction = exactWidth - static_cast<double>(width);
  if (width >= kContentWidth) {
    width = kContentWidth;
    edgeFraction = 0.0;
  }
  if (!bar.colorSet || !sameColor(bar.fillColor, fillColor)) {
    lv_obj_set_style_bg_color(bar.fill, color(fillColor), 0);
    lv_obj_set_style_bg_color(bar.edge, color(fillColor), 0);
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

  const lv_opa_t edgeOpacity = static_cast<lv_opa_t>(std::lround(
      static_cast<double>(opacity) * edgeFraction));
  const bool showEdge = width < kContentWidth && edgeOpacity > LV_OPA_TRANSP;
  if (!showEdge) {
    if (!bar.edgeHidden) {
      lv_obj_add_flag(bar.edge, LV_OBJ_FLAG_HIDDEN);
      bar.edgeHidden = true;
    }
  } else {
    if (bar.edgeHidden) {
      lv_obj_clear_flag(bar.edge, LV_OBJ_FLAG_HIDDEN);
      bar.edgeHidden = false;
    }
    const std::int32_t edgeX = kContentX + width;
    if (bar.edgeX != edgeX) {
      lv_obj_set_x(bar.edge, edgeX);
      bar.edgeX = edgeX;
    }
    if (bar.edgeOpacity != edgeOpacity) {
      lv_obj_set_style_opa(bar.edge, edgeOpacity, 0);
      bar.edgeOpacity = edgeOpacity;
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
    case TemperatureState::hot:
      return "CALIENTE";
    case TemperatureState::veryHot:
      return "MUY CALIENTE";
  }
  return "FALLO";
}

}  // namespace

void createOilGaugeUi(lv_obj_t* screen) {
  gUi = {};
  gUi.screen = screen;

  lv_obj_remove_style_all(screen);
  lv_obj_set_size(screen, kCanvasWidth, kCanvasWidth);
  lv_obj_set_style_bg_color(screen, color(kBlack), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t* divider = createSolid(screen, 0, kHalfHeight - 1, 480, 1, 0);
  lv_obj_set_style_bg_color(divider, color(kLine), 0);

  createLabel(screen,
              CONFIG_OIL_GAUGE_DEMO_MODE ? "DEMO" : "CAL PENDIENTE",
              180,
              7,
              120,
              14,
              &oil_font_ui_12,
              color(kSecondary),
              LV_TEXT_ALIGN_CENTER);

  createLabel(screen,
              "PRESIÓN ACEITE",
              kContentX,
              34,
              190,
              20,
              &oil_font_ui_16,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  gUi.pressureState = createLabel(screen,
                                  "OK",
                                  230,
                                  28,
                                  212,
                                  32,
                                  &oil_font_ui_24,
                                  color(kSecondary),
                                  LV_TEXT_ALIGN_RIGHT);
  gUi.pressureIcon = createPressureIcon(screen);
  gUi.pressureValue = createLabel(screen,
                                  "61",
                                  120,
                                  75,
                                  240,
                                  106,
                                  &oil_font_value_96,
                                  color(kPrimary),
                                  LV_TEXT_ALIGN_CENTER);
  lv_obj_set_style_text_letter_space(gUi.pressureValue, -5, 0);
  createLabel(screen,
              "PSI",
              340,
              119,
              70,
              32,
              &lv_font_montserrat_24,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);

  static constexpr double kPressureTicks[] = {0.067, 0.10, 0.533};
  gUi.pressureBar =
      createBar(screen, 184, kPressureTicks, std::size(kPressureTicks));
  createLabel(screen,
              "Alerta ≤10 PSI",
              kContentX,
              201,
              180,
              16,
              &oil_font_ui_12,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  createLabel(screen,
              "OK: 15–80 PSI",
              262,
              201,
              180,
              16,
              &oil_font_ui_12,
              color(kSecondary),
              LV_TEXT_ALIGN_RIGHT);

  createLabel(screen,
              "TEMPERATURA ACEITE",
              kContentX,
              274,
              230,
              20,
              &oil_font_ui_16,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  gUi.temperatureState = createLabel(screen,
                                     "ÓPTIMO",
                                     230,
                                     268,
                                     212,
                                     32,
                                     &oil_font_ui_24,
                                     color(kSecondary),
                                     LV_TEXT_ALIGN_RIGHT);
  gUi.temperatureIcon = createTemperatureIcon(screen);
  gUi.temperatureValue = createLabel(screen,
                                     "92",
                                     120,
                                     315,
                                     240,
                                     106,
                                     &oil_font_value_96,
                                     color(kPrimary),
                                     LV_TEXT_ALIGN_CENTER);
  lv_obj_set_style_text_letter_space(gUi.temperatureValue, -5, 0);
  createLabel(screen,
              "°C",
              340,
              359,
              70,
              32,
              &lv_font_montserrat_24,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);

  static constexpr double kTemperatureTicks[] = {
      0.080, 0.284, 0.455, 0.500, 0.568};
  gUi.temperatureBar = createBar(
      screen, 424, kTemperatureTicks, std::size(kTemperatureTicks));
  createLabel(screen,
              "Óptimo desde 75 °C",
              kContentX,
              441,
              200,
              16,
              &oil_font_ui_12,
              color(kSecondary),
              LV_TEXT_ALIGN_LEFT);
  createLabel(screen,
              ">94 °C caliente",
              252,
              441,
              190,
              16,
              &oil_font_ui_12,
              color(kSecondary),
              LV_TEXT_ALIGN_RIGHT);

  gUi.created = true;
}

void updateOilGaugeUi(const ConvertedValue& pressure,
                      const ConvertedValue& temperature,
                      const EngineState& engine,
                      bool blinkPhaseOn,
                      bool reducedMotion) {
  if (!gUi.created) {
    return;
  }

  const DisplayState state = evaluateDisplayState(
      pressure, temperature, engine, blinkPhaseOn, reducedMotion);
  char pressureText[8];
  if (pressure.valid()) {
    std::snprintf(pressureText,
                  sizeof(pressureText),
                  "%d",
                  static_cast<int>(std::lround(pressure.value)));
  } else {
    std::snprintf(pressureText, sizeof(pressureText), "--");
  }
  setLabelTextIfChanged(
      gUi.pressureValue, gUi.pressureValueText, pressureText);
  setLabelTextIfChanged(gUi.pressureState,
                        gUi.pressureStateText,
                        pressureLabel(state.pressure));
  if (!gUi.pressureColorSet ||
      !sameColor(gUi.pressureColor, state.pressureColor)) {
    const lv_color_t pressureColor = color(state.pressureColor);
    lv_obj_set_style_text_color(gUi.pressureState, pressureColor, 0);
    setIconColor(gUi.pressureIcon, pressureColor);
    gUi.pressureColor = state.pressureColor;
    gUi.pressureColorSet = true;
  }

  const bool warning = state.pressure == PressureState::warning;
  const lv_opa_t attentionOpacity =
      warning && !state.pressureAttentionVisible ? LV_OPA_20 : LV_OPA_COVER;
  if (gUi.pressureAttentionOpacity != attentionOpacity) {
    lv_obj_set_style_opa(gUi.pressureIcon, attentionOpacity, 0);
    lv_obj_set_style_text_opa(gUi.pressureState, attentionOpacity, 0);
    gUi.pressureAttentionOpacity = attentionOpacity;
  }
  updateBar(gUi.pressureBar,
            state.pressureBarFraction,
            state.pressureColor,
            attentionOpacity);

  char temperatureText[8];
  if (!temperature.valid()) {
    std::snprintf(temperatureText, sizeof(temperatureText), "--");
  } else if (state.showTemperatureBelowRange) {
    std::snprintf(temperatureText, sizeof(temperatureText), "<50");
  } else {
    std::snprintf(temperatureText,
                  sizeof(temperatureText),
                  "%d",
                  static_cast<int>(std::lround(temperature.value)));
  }
  setLabelTextIfChanged(
      gUi.temperatureValue, gUi.temperatureValueText, temperatureText);
  setLabelTextIfChanged(gUi.temperatureState,
                        gUi.temperatureStateText,
                        temperatureLabel(state.temperature));
  if (!gUi.temperatureColorSet ||
      !sameColor(gUi.temperatureColor, state.temperatureColor)) {
    const lv_color_t temperatureColorValue = color(state.temperatureColor);
    lv_obj_set_style_text_color(
        gUi.temperatureState, temperatureColorValue, 0);
    setIconColor(gUi.temperatureIcon, temperatureColorValue);
    gUi.temperatureColor = state.temperatureColor;
    gUi.temperatureColorSet = true;
  }
  updateBar(gUi.temperatureBar,
            state.temperatureBarFraction,
            state.temperatureColor,
            LV_OPA_COVER);
}

}  // namespace oilgauge
