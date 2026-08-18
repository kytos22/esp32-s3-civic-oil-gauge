cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "src")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(civic_oil_gauge)

# Keep the registry BSP immutable while selecting the explicitly approved panel
# clock for this project. The forced include sees the CO5300 defaults first and
# then replaces only the QSPI IO configuration macro used by Waveshare's BSP.
idf_component_get_property(oil_gauge_waveshare_bsp
                           waveshare__esp32_s3_touch_amoled_2_16
                           COMPONENT_LIB)
target_compile_options(${oil_gauge_waveshare_bsp} PRIVATE
                       "-include${CMAKE_SOURCE_DIR}/include/display_clock_override.h")
