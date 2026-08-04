# Sprint 3 — Interactive README simulator entry

- Scope: turn the static README preview into a clear entry point to the approved
  pressure/RPM/temperature simulator without claiming JavaScript can execute
  inside GitHub README rendering.
- Acceptance:
  - English and Spanish preview images open the same live HTTPS simulator;
  - both READMEs provide an explicit, localized simulator link and explain the
    GitHub inline-JavaScript limitation;
  - GitHub Pages publishes the existing standalone HTML from `docs/` without a
    copied simulator source;
  - static checks prove the three range inputs and their input listeners remain;
  - the deployed URL returns the expected simulator document;
  - browser-driven slider behavior is reported only when actually driven.
- Status: in progress — requested by Marcos on 2026-08-04

## Slices

| Slice | Status | Test point result | Notes |
| --- | --- | --- | --- |
| 3.1 Bilingual interactive entry | in progress | local checks pending | Linked preview plus explicit CTA |
| 3.2 GitHub Pages publication | pending | external deployment pending | Serve `main:/docs` |
| 3.3 Deployed interaction verification | pending | browser driver bootstrap blocked by WSL path issue | Static/deployed response checks remain driveable |
