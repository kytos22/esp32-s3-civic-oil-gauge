#include "display_runtime.h"

#include "display_clock_profile.h"
#include "esp_err.h"
#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "bsp/touch.h"
#include "esp_lcd_panel_io.h"
#include "esp_log.h"
#include "esp_lv_adapter.h"

namespace oilgauge {

namespace {

constexpr char kTag[] = "display_runtime";

void roundInvalidationToPanelAlignment(lv_event_t* event) {
  lv_area_t* area = static_cast<lv_area_t*>(lv_event_get_param(event));
  area->x1 = (area->x1 >> 1) << 1;
  area->y1 = (area->y1 >> 1) << 1;
  area->x2 = ((area->x2 >> 1) << 1) + 1;
  area->y2 = ((area->y2 >> 1) << 1) + 1;
}

}  // namespace

OilDisplayRuntime startOilDisplayRuntime() {
  OilDisplayRuntime runtime;

  ESP_LOGI(kTag,
           "CO5300 QSPI clock request: %u MHz",
           static_cast<unsigned>(OIL_GAUGE_DISPLAY_QSPI_HZ / 1'000'000));

  const esp_lv_adapter_config_t adapterConfig{
      .task_stack_size = ESP_LV_ADAPTER_DEFAULT_STACK_SIZE,
      .task_priority = ESP_LV_ADAPTER_DEFAULT_TASK_PRIORITY,
      .task_core_id = ESP_LV_ADAPTER_DEFAULT_TASK_CORE_ID,
      .tick_period_ms = ESP_LV_ADAPTER_DEFAULT_TICK_PERIOD_MS,
      .task_min_delay_ms = ESP_LV_ADAPTER_DEFAULT_TASK_MIN_DELAY_MS,
      .task_max_delay_ms = ESP_LV_ADAPTER_DEFAULT_TASK_MAX_DELAY_MS,
      .stack_in_psram = false,
      .auto_sleep = {
          .enable = false,
          .mode = ESP_LV_ADAPTER_AUTO_SLEEP_MODE_DISABLED,
          .idle_timeout_ms = ESP_LV_ADAPTER_DEFAULT_AUTO_SLEEP_TIMEOUT_MS,
          .callbacks = {},
      },
  };
  if (esp_lv_adapter_init(&adapterConfig) != ESP_OK) {
    ESP_LOGE(kTag, "LVGL adapter initialization failed");
    return runtime;
  }

  const bsp_display_config_t panelConfig{
      .max_transfer_sz =
          BSP_LCD_H_RES * BSP_LCD_V_RES * BSP_LCD_BITS_PER_PIXEL / 8,
  };
  esp_lcd_panel_handle_t panel = nullptr;
  esp_lcd_panel_io_handle_t panelIo = nullptr;
  if (bsp_display_new(&panelConfig, &panel, &panelIo) != ESP_OK) {
    ESP_LOGE(kTag, "CO5300 panel initialization failed");
    return runtime;
  }

  esp_lv_adapter_display_config_t displayConfig{};
  displayConfig.panel = panel;
  displayConfig.panel_io = panelIo;
  displayConfig.profile.interface = ESP_LV_ADAPTER_PANEL_IF_OTHER;
  displayConfig.profile.rotation = ESP_LV_ADAPTER_ROTATE_0;
  displayConfig.profile.hor_res = BSP_LCD_H_RES;
  displayConfig.profile.ver_res = BSP_LCD_V_RES;
  displayConfig.profile.buffer_height = BSP_LCD_V_RES;
  displayConfig.profile.use_psram = true;
  displayConfig.profile.enable_ppa_accel = false;
  displayConfig.profile.require_double_buffer = true;
  displayConfig.profile.mono_layout = ESP_LV_ADAPTER_MONO_LAYOUT_NONE;
  displayConfig.tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_NONE;
  displayConfig.te_sync.gpio_num = -1;

  runtime.display = esp_lv_adapter_register_display(&displayConfig);
  if (runtime.display == nullptr) {
    ESP_LOGE(kTag, "Full-frame LVGL display registration failed");
    return runtime;
  }
  lv_display_add_event_cb(runtime.display,
                          roundInvalidationToPanelAlignment,
                          LV_EVENT_INVALIDATE_AREA,
                          nullptr);

  bsp_display_cfg_t touchBspConfig{};
  touchBspConfig.touch_flags.swap_xy = 1;
  touchBspConfig.touch_flags.mirror_x = 0;
  touchBspConfig.touch_flags.mirror_y = 1;
  esp_lcd_touch_handle_t touch = nullptr;
  if (bsp_touch_new(&touchBspConfig, &touch) != ESP_OK) {
    ESP_LOGE(kTag, "Touch initialization failed");
    return runtime;
  }
  const esp_lv_adapter_touch_config_t touchConfig{
      .disp = runtime.display,
      .handle = touch,
      .scale = {.x = 1.0F, .y = 1.0F},
      .multi_touch = {
          .mode = ESP_LV_ADAPTER_TOUCH_MODE_SINGLE,
          .pointers = 1,
      },
      .callbacks = {},
  };
  runtime.input = esp_lv_adapter_register_touch(&touchConfig);
  if (runtime.input == nullptr) {
    ESP_LOGE(kTag, "LVGL touch registration failed");
    return runtime;
  }

  if (bsp_display_brightness_init() != ESP_OK ||
      esp_lv_adapter_start() != ESP_OK) {
    ESP_LOGE(kTag, "Display runtime start failed");
    runtime = {};
  }
  return runtime;
}

}  // namespace oilgauge
