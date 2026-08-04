# Adoption Audit — Civic ESP32 Oil Gauge

> Keel v5.3.2 gap audit, 2026-07-30. No firmware was changed during adoption.
> Remediation Sprint 0 was approved in full in D-012 and resolved the software-only
> "fix now" items; hardware findings remain open by design.
> Codex has no project markdown verifier agents, so the required dimensions were checked inline.

## Executive assessment

The project has a sound safety premise: calibration is invalid by default and the
MTX-D remains the reference. The largest immediate risk is not an invented curve
in current firmware; it is the inability to reproduce the software verification
now, combined with a temporary alarm/UI implementation that does not yet satisfy
the approved contract. No current file scan found a secret-shaped credential.

## Findings

| ID | Dimension | Severity | Finding and evidence | Keel/contract gap |
|---|---|---|---|---|
| A-01 | Hygiene/state | high | `.git` is an empty directory and `git rev-parse` reports “not a git repository.” No safe commits, hook verification, or handoff identity are possible. | Valid repository and verified state gates |
| A-02 | Testability | high | `pio test -e native` returned `pio: command not found`; `pio run` cannot be repeated here. Historical 2026-07-28 results remain historical only. | Verification claimed only from current command evidence |
| A-03 | Functional safety | critical before real use | `evaluateAlarms()` uses pressure `<15 PSI` without engine/RPM state. It would alarm with the engine stopped if fed valid values. | AC-05/06 and D-007 |
| A-04 | Functional/UI | high | `src/main.cpp` is a temporary text/bar renderer with no approved icons, semantic states, continuous temperature colors, `<50`, or reduced-motion behavior. | D-006, `docs/UI_DESIGN.md`, AC-06–12 |
| A-05 | Calibration | expected blocker | Both calibrations are intentionally invalid. Sensor pinout, excitation, pressure function, and R/T curve remain unknown. | AC-13–18; no cutover until evidence |
| A-06 | Electrical safety | critical before vehicle use | Protected power, reversible front end, clamps, star ground, and USB/back-feed behavior exist only as design/manual controls. | Threat model controls remain MANUAL/TO BUILD/VERIFY |
| A-07 | Hardware | expected blocker | Display has not arrived; no physical display, touch, current, ADC, MTS, thermal, reset, or vehicle evidence exists. | AC-20 and arrival checklist |
| A-08 | Test coverage | high | Eight core tests exist, but no RPM-aware alarm boundary tests, approved renderer-state tests, ADC integration tests, MTS parser tests, or AC-to-evidence ledger exist. | `docs/05-test-points.md`, driven-criterion rule |
| A-09 | Environment doctor | medium | No `scripts/keel-doctor`; missing Git/PlatformIO can only be discovered ad hoc. | MANIFEST Phase 5/adoption requirement |
| A-10 | Project verifier | medium | No `scripts/keel-verify`; marked paths, links, conformance states, cited commands, and orphan docs are not mechanically checked. | Anti-pattern self-audit |
| A-11 | Continuation safety | medium | No `scripts/keel-handoff-verify`; current invalid Git prevents the five courier checks. | Keel continuation contract |
| A-12 | Development trace | medium | No sprint files or `docs/05-test-points.md`; the existing code predates Keel and has no slice/test-point history. | Phase 5 traceability |
| A-13 | Design handoff | resolved in Sprint 0 | Approved files now live byte-for-byte in `docs/design/references/`; the adopted brief/handoff and BUILD-SPEC map states to implementation evidence. | MANIFEST UI rows |
| A-14 | Visual source pipeline | medium | Editable fragment, standalone HTML, and PNG exist, but no generator/check proves standalone/capture match the fragment. | Single source of truth / generated artifact consumer |
| A-15 | Static analysis | medium | No formatter, linter, static analyzer, or warning-as-error command is configured. | Declared tooling must run and block |
| A-16 | API/docs | low now, medium if shared | `docs/api/INDEX.md` inventories 15 core surfaces, while only eight behavior tests exist and full per-surface docs are deferred. | Progressive backfill on next touch |
| A-17 | Language consistency | low | Two Spanish comments remain in `include/calibration_config.h`; adoption did not change code. | English source-comment convention |
| A-18 | Version/license | low until binary release | PolyForm Noncommercial 1.0.0 and required project notice now exist; no project version, complete dependency-notice set or package audit exists. | Complete remaining items before Phase 7 binary release |
| A-19 | Accessibility/glanceability | high before driving | No real AMOLED daylight/night, color, motion, glare, target-size, or fault-legibility pass exists. | Honest embedded-display accessibility record |
| A-20 | RPM source | high | WiCAN/CAN/other engine-state source is not selected or validated. Oil pressure warning cannot meet its contract without it. | Integration trust boundary and AC-05/06 |

## Known-traps self-audit

| Check | Result/evidence |
|---|---|
| Declared tools actually run | fail — PlatformIO command unavailable |
| Cited commands exist | partial — commands are documented and environment configured, but the executable is absent |
| Version touchpoints agree | n/a — no version exists |
| `[E]` paths exist | manual spot-check passes; no `keel-verify` yet |
| Documented extension/public surfaces have tests | partial — 8 tests for 15 indexed surfaces |
| Generated artifacts have consumers | fail/unknown — prototype fragment→standalone→PNG pipeline is undocumented |
| Test-point rows carry evidence | fail — test-point file absent |
| Suppression count | pass — search found no suppressions in product/test code |
| Omissions recorded | pass — decisions and threat-model “Not defended” table |
| Present-tense controls are evidenced | pass after adoption rewrite; planned controls carry states |
| One authoritative artifact per fact | partial — UI source direction needs a generator/check |
| Docs reachable/internal links | partial — archive indexed; no mechanical link checker |
| Every user-visible AC is driven or tagged | fail — renderer and hardware drivers absent |
| Doctor passes | fail — doctor absent and environment blockers known |
| Conformance sweep complete | pass as a sweep; eleven rows remain `missing` pending user decision |
| Library dependencies backed by decisions | partial — architecture choice recorded, full license/supply-chain decisions pending public distribution |

## Proposed prioritization

### Fix now — Remediation Sprint 0, before hardware arrival — completed

1. Valid local Git repository initialized without file loss.
2. PlatformIO Core 6.1.19 installed in a project venv with isolated WSL state.
3. `scripts/keel-doctor`, `scripts/keel-verify`, and `scripts/keel-handoff-verify` created.
4. `.gitattributes`, `docs/sprints/`, and `docs/05-test-points.md` created.
5. Adopted design brief/BUILD-SPEC created; references moved byte-for-byte.
6. RPM-aware pressure-state API specified, documented, and tested.
7. Deterministic pressure/temperature state, color, motion, and geometry gates added.

### Fix when the area is touched

- Normalize the visual-reference directory only through a recorded move/update, never a duplicate copy.
- Add full API docs when each core surface is changed.
- Replace the two Spanish code comments when `calibration_config.h` is next changed.
- Add a debug logging switch during the first hardware integration slice.
- Add static analysis in the same change that makes it pass and blocking.
- Project license and required notice were added by D-028; add version and complete
  third-party notices only when a binary distribution/release decision is made.

### Accepted/deferred by current project decisions

- No website, client budget, touch configuration, automatic chat chaining, MCP registration, or assistant agents.
- No end-user guide until a hardware-validated release candidate.
- Hardware, calibration, and vehicle checks wait for the physical board and explicit per-action authorization.
- The separate WiCAN/CAN display remains outside this firmware except for a narrowly specified engine-running input.

## Adoption closure

D-012 accepted the complete batch. `docs/keel-conformance.md` now records 30 present,
0 missing, 24 not applicable/not yet due, and 0 declined requirements.
