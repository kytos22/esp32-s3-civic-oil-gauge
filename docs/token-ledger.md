# Token Ledger — Civic ESP32 Oil Gauge

> Actual token usage. One row per working session, appended at session end.
> Values are measured when the environment exposes them; otherwise estimated and rounded up.

## Sessions

| Date | Phase/sprint | Model(s) | Input tokens | Output tokens | Method | Notes |
|---|---|---|---:|---:|---|---|
| 2026-07-30 | Keel adoption | Codex session model | 120000 | 32000 | estimated, rounded up from loaded references and artifact volume | Full adoption inventory, translation, specification, conformance sweep, and audit; environment exposes no exact counter |
| 2026-07-30 | Phase 5 / Sprint 0 remediation | Codex session model | 65000 | 18000 | estimated, rounded up; environment exposes no exact counter | Git/toolchain remediation, adopted design contract, state core, tests, full build, and self-audit |
| 2026-07-30 | Phase 5 / Sprint 0 close | Codex session model | 18000 | 5000 | estimated, rounded up; environment exposes no exact counter | Local author setup, confidential-data recheck, evidence commit, close-out, and courier verification |
| 2026-08-04 | Phase 5 / graphics and demo firmware | Codex session model | 90000 | 22000 | estimated, rounded up; environment exposes no exact counter | Current ESP-IDF/Waveshare research, native LVGL 50/50 renderer, embedded fonts, demo sequence, complete build, native tests, and Keel close-out |
| 2026-08-04 | Phase 5 / hardware arrival and demo flash | Codex session model | 110000 | 28000 | estimated, rounded up from loaded Keel references, command evidence, and artifact volume; environment exposes no exact counter | Exact-board authorization, verified flash, driven boot diagnosis, BSP lock correction, reflash, clean bounded boot capture, documentation, and handover |
| 2026-08-04 | Phase 5 / physical text-rendering correction | Codex session model | 70000 | 18000 | estimated, rounded up from source diagnosis, LVGL evidence, full rebuilds, hardware flash, and documentation; environment exposes no exact counter | Diagnosed compressed-font draw-unit race, serialized LVGL rendering, added an effective-config gate, passed 12/12 native tests and full build, flashed exact board, and captured a clean bounded boot |
| 2026-08-04 | Phase 5 / fluid demo and FPS instrumentation | Codex session model | 65000 | 17000 | estimated, rounded up from implementation, font generation, native and firmware validation, hardware discovery, and documentation; environment exposes no exact counter | Added continuous interpolation, 16 ms LVGL refresh, completed-frame FPS logging, uncompressed 24 px Spanish status font, 14/14 native tests, and an exact committed image; hardware write stopped because the board was absent |
| 2026-08-04 | Phase 5 / sustained FPS hardware proof | Codex session model | 30000 | 9000 | estimated, rounded up from Windows USB identity checks, two exact-board flashes, bounded serial captures, optimization, rebuilds, and evidence reconciliation; environment exposes no exact counter | Measured the initial 9–58 FPS failure, added linear fractional-pixel bar motion and 15 ms headroom, then proved eight consecutive completed-frame windows at 65–67 FPS on the exact board |
| 2026-08-04 | Phase 5 / Sprint 2 repository productization | Codex session model | 45000 | 14000 | estimated, rounded up from Keel/reference inspection, bilingual authoring, privacy remediation, test/build execution and repository verification; environment exposes no exact counter | Used boost-gauge commit `9ca4340` only as a structural reference, added bilingual public entry points and versioned assets/firmware contracts, omitted golden version, sanitized the tracked snapshot and passed 14/14 native tests plus the full ESP-IDF build |
| 2026-08-04 | Phase 5 / Sprint 3 README simulator and GIF | Codex session model | 45000 | 13000 | estimated, rounded up from Pages deployment, HTML integrity verification, 205-frame rendering, visual inspection and documentation; environment exposes no exact counter | Published the approved simulator through GitHub Pages, linked both README previews, then replaced the stepped preview with a 50 FPS smooth-step GIF rendered from real HTML states while preserving the live simulator destination |
| 2026-08-04 | Phase 5 / Sprint 4 noncommercial license | Codex session model | 8000 | 3000 | estimated, rounded up from official-license research, exact-text comparison, bilingual summaries and repository verification; environment exposes no exact counter | Selected PolyForm Noncommercial 1.0.0 so improvements and redistribution are permitted for noncommercial purposes while commercial use remains excluded; added the official terms and required notice |

Running total: approximately 666000 input / 179000 output.

## Final reconciliation

- Total tokens by model: pending release.
- Cost at verified prices: pending release.
- Estimate vs actual: pending release.
- Lesson for future estimates: pending release.
