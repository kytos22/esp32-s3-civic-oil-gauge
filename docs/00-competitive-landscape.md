# Competitive Landscape — Civic ESP32 Oil Gauge

> Research snapshot: 2026-07-30. Prices are vendor snapshots, not purchasing advice.

## Competitor inventory

| Product | Category / status | Price found | Relevant capabilities | Sources |
|---|---|---:|---|---|
| Innovate MTX-D 39130 | Active 52 mm dual oil gauge; installed reference unit | USD 201.59 sale / 223.99 list | Simultaneous pressure and temperature, programmable pressure/temperature warnings, configurable LED ring, PSI/bar, replaceable bezel/face, MTS/LogWorks logging | [product](https://www.innovatemotorsports.com/mtx-d-oil-pressure-temperature.html), [manual](https://www.innovatemotorsports.com/wp/content/uploads/2022/05/MTX-D-Oil-Press-Temp.pdf) |
| AEM Classic/X-Series | Active 52 mm single-variable gauges | Pressure USD 249.95 Classic or 276.95 X-Series; temperature USD 224.95 | Fast digital value, LED sweep, auto dimming on Classic temperature, sensor kits, 0–5 V output on Classic, 150 PSI pressure range | [Classic range](https://www.aemelectronics.com/products/dashes_and_gauges/aem_performance_gauges/classic_series_gauges/), [X-Series pressure](https://www.aemelectronics.com/products/dashes_and_gauges/aem_performance_gauges/x_series_gauges/parts/30-0307) |
| Defi ADVANCE FD / Sports Display F | Active configurable motorsport display ecosystem | Not determined | Multiple layouts/zones, bars and graphs, oil pressure and temperature, warm-up mode, warning thresholds, buzzer, sensor/open-short diagnosis, OBD/CAN paths on supported configurations | [ADVANCE FD manual](https://www.defi-shop.com/wp-content/uploads/2025/04/manual_fd_e.pdf), [Sports Display F update manual](https://www.defi-shop.com/media/manual_dsdf_update4.0_e.pdf) |
| Banks iDash Pro/Data Pro | Active configurable 52 mm CAN/OBD display | Not determined | User-selectable data and layouts, OBD-II/CAN acquisition, many parameters, logging on Data variants, external sensor modules and configurable alerts | [product](https://bankspower.com/products/banks-idash-pro-data-pro-digital-datalogger), [FAQ](https://bankspower.com/pages/faq-frequently-asked-questions) |
| ECUMaster ADU | Active full motorsport dashboard; adjacent rather than direct competitor | Not determined | Multi-page layouts, logging, warning overlays independent of active page, broad CAN/analog integration | [product](https://www.ecumaster.com/products/adu/) |

## Unified feature list

| Functionality | MTX-D | AEM | Defi | Banks | ECUMaster |
|---|:---:|:---:|:---:|:---:|:---:|
| Large live numeric value | yes | yes | yes | yes | yes |
| Pressure and temperature on one screen | yes | no, separate gauges | yes | configurable | configurable |
| Programmable warning thresholds | yes | model-dependent | yes | yes | yes |
| Warm-up/cold state | limited LED configuration | no evidence found | yes | configurable | configurable |
| Sensor fault/open-short indication | not determined | not determined | yes | depends on source | configurable |
| Auto/manual dimming | illumination input | yes on Classic temperature | yes | yes | yes |
| Logging | MTS/LogWorks | external 0–5 V/logger | internal/app depending on model | Data models | yes |
| User-selectable units | PSI/bar; °F/°C through programming | model-specific | yes | yes | yes |
| User-selectable layouts | no | no | yes | yes | yes |
| Peak/minimum recall | LogWorks/limited gauge behavior | model-dependent | peak/record/playback | logging-derived | configurable |
| CAN/OBD acquisition | no native OBD | via external modules | supported configurations | core feature | core feature |

## External-demand evidence

Evidence is limited and is not used to widen v1 automatically:

- Users ask how to display minimum/maximum values on ECUMaster ADU variables, including oil pressure:
  [ECUMaster community thread](https://community.ecumaster.com/t/min-max-values/5562).
- Users need explicit oil-temperature sensor calibration support on configurable dashboards:
  [ECUMaster community thread](https://community.ecumaster.com/t/oil-temperature-sensor-calibration/4366).

No reliable demand evidence was found for AI, cloud, or touch features in this category.

## Competitive confrontation and accepted roadmap

| Functionality | Proposed disposition | Reason |
|---|---|---|
| Dual simultaneous oil values | v1 | Core advantage of the installed MTX-D and the approved design |
| Clear numeric value and units | v1 | Table stakes |
| Warm-up/cold state | v1 | Protects the engine and prevents false precision |
| Pressure warning gated by engine-running state | v1 | Stronger than a fixed low-pressure threshold |
| Temperature warning and continuous color progression | v1 | Approved design and table stakes |
| Sensor/open/short fault state | v1 | Fail-safe requirement |
| Calibration logging/raw ADC view | v1 development tool | Necessary because the Innovate curves are unpublished |
| Dimming/headlight input | Later, before permanent installation | Useful but secondary to measurement correctness |
| Peak/minimum recall | Later | Demand exists, but it does not protect the engine better in v1 |
| Permanent trip logging | Later | LogWorks and development CSV cover calibration first |
| User-selectable layouts | Never for the oil screen unless the user reverses D-006 | The approved 50/50 layout is binding |
| Touch configuration while driving | Never | Unnecessary distraction and safety risk |
| CAN/OBD dashboard breadth | Separate second-display project | The user intends a second identical screen for CAN data |
| AI/MCP layer | Dropped as forced filler | Adds no value to a deterministic safety instrument |
