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
- Status: complete with one recorded browser-driver limitation — requested and
  published by Marcos on 2026-08-04

## Slices

| Slice | Status | Test point result | Notes |
| --- | --- | --- | --- |
| 3.1 Bilingual interactive entry | complete | verifier confirms two links per README and three range controls/listeners | Linked preview plus explicit CTA |
| 3.2 GitHub Pages publication | complete | Pages build for `61d3dc3` reports `built`; HTTPS enforced | Serves `main:/docs` |
| 3.3 Deployed interaction verification | partial | downloaded artifact is byte-identical to approved source; browser slider-driving is `PLATFORM-IMPOSSIBLE` in this session | Browser bootstrap rejects the WSL workspace path containing spaces |

## Deployment evidence

- Live URL:
  `https://kytos22.github.io/esp32-s3-civic-oil-gauge/design/references/oil-gauge-design.html`
- GitHub Pages: public, HTTPS enforced, source `main:/docs`, build commit
  `61d3dc3fdcea242eb8af79f84bedf0ae884e082b`, status `built`.
- Deployed and local standalone HTML: 47,509 bytes and identical SHA-256
  `b96bddee2599f8fd4ddfcc233e6ba87aec415dcbc7c38567554197cded62e9e1`.
- Static runtime contract: pressure, RPM and temperature range IDs plus the shared
  `input` event listener are present in the deployed file.
- Unrun leg: `PLATFORM-IMPOSSIBLE` browser slider-driving due to the Browser
  bootstrap rejection of this WSL workspace path with spaces.
