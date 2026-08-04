# Sprint 2 — Public repository productization

- Scope: turn the existing development repository into a clear, bilingual,
  release-ready oil-gauge repository using only structural lessons from the Civic
  boost-gauge repository, without copying its project data or adding a golden
  version mechanism.
- Acceptance:
  - root English and Spanish READMEs truthfully describe the oil-gauge project;
  - public navigation exposes features, hardware, safety, build, documentation and
    future firmware downloads;
  - `assets/` and `firmware/` have explicit oil-only ownership and release rules;
  - future packages use immutable `firmware/<version>/` directories and hashes;
  - no `GOLDEN_VERSION*`, generated build tree, local log or exact device identifier
    exists in the publishable snapshot;
  - the complete source tree still passes native tests, ESP-IDF build and Keel
    consistency checks;
  - remote creation waits for the user's explicit public/private choice; Marcos
    selected public visibility on 2026-08-04.
- Status: complete — requested and published by Marcos on 2026-08-04

## Slices

| Slice | Status | Test point result | Notes |
| --- | --- | --- | --- |
| 2.1 Reference-only structure audit | complete | source and local checkout both at `9ca4340` | Structural patterns recorded; project data excluded |
| 2.2 Bilingual public entry points | complete | reciprocal links and content audit pass | English base plus Spanish reader entry point |
| 2.3 Assets and release namespaces | complete | required-path, link and release-contract audit pass | No production binary is claimed |
| 2.4 Publishable-snapshot privacy gate | complete | no golden/vendor/simulator/generated path or exact MAC-like identifier in tracked snapshot | Current historical commits must not be pushed publicly |
| 2.5 Public remote creation | complete | GitHub reports `PUBLIC`, default branch `main`, one sanitized root before this completion record | `https://github.com/kytos22/esp32-s3-civic-oil-gauge` |

## Verification evidence

- Reference remote and local checkout both resolve to boost-gauge commit `9ca4340`.
- `./scripts/pio test -e native`: 14/14 passed on 2026-08-04.
- `./scripts/idf build`: complete 676,224-byte ESP32-S3 image built on 2026-08-04.
- `./scripts/keel-verify`: 27/27 acceptance criteria covered; bilingual,
  publishable-tree, link, document, safety and code-map gates pass.
- GitHub post-publish verification reports public visibility, `main` as the only
  branch and sanitized root `5ad0a4d` as the sole commit before this living-state
  completion record.
