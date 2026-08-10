# Estimate — Civic ESP32 Oil Gauge

> Internal working estimate. AI-time based: AI session hours plus vibe-coder
> supervision. Never based on traditional human-team development time.

## Estimate v1 — adoption/remediation plan — 2026-07-30

### Scope basis

- Six functional areas, two branching flows, one 480×480 screen with at least ten semantic/fault states.
- Four main workstreams: environment/repository, renderer, acquisition/calibration, and vehicle-safe validation.
- One separate CAN display explicitly excluded.

### AI working hours

| Segment | AI hours | Basis |
|---|---:|---|
| Keel adoption, research, and as-built specification | 3–5 h | Existing firmware/docs plus competitor and conformance sweep |
| Environment doctor, verification scripts, and repository hygiene | 1.5–3 h | PlatformIO/Git recovery and Keel gates |
| Approved 50/50 renderer and deterministic state tests | 2–4 h | One screen, many states, captured comparison |
| Direct acquisition + calibration tooling | 3–6 h | ADC, pressure, thermistor, MTS/reference capture |
| Hardware/vehicle evidence analysis and fixes | 2–5 h | Multiple bench and supervised vehicle iterations |
| Documentation consolidation and release gate | 1–2 h | Architecture, usage, safety, test evidence |
| **Total before contingency** | **12.5–25 h** | |

### Vibe-coder supervision

| Segment | What the developer does | Hours |
|---|---|---:|
| Decisions and purchasing | Approve gates, select parts, provide photos | 1–2 h |
| Bench assembly | Build reversible harness/front end and make guided measurements | 3–6 h |
| Hardware arrival tests | Connect USB/I²C safely, report observations, manage physical setup | 1.5–3 h |
| Vehicle validation | Supervised cold/hot/multiple-speed captures over at least three drives | 3–6 h |
| Final cutover | Inspect installation and approve or reject replacement | 1–2 h |
| **Total before contingency** | | **9.5–19 h** |

### Contingency

Add 25% for undocumented curves, connector identification, power noise, MTS decoding, and renderer/display-driver differences:

- AI working time with contingency: approximately **16–31 h**.
- Vibe-coder supervision with contingency: approximately **12–24 h**.

### Calendar delivery

Not estimated until the display arrives and the user's weekly availability is known. Hardware shipping and three-drive validation are elapsed-time constraints, not AI working time.

### AI cost

Planning assumption: subscription/seat access, therefore no separate marginal token cost. If this project moves to metered API use, append a new estimate using current official pricing.

### Assumptions and risks

- Waveshare board and bought parts are not yet physically available.
- PlatformIO currently cannot run in WSL.
- Direct sensor calibration may fail; in that case the original MTX-D remains
  installed and the replacement gauge does not enter service.
- Final automotive PCB/enclosure certification is excluded.
- The CAN/OBD display is excluded and planned separately.
