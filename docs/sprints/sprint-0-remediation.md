# Sprint 0 — Reproducible pre-hardware foundation

- Scope: close the eleven Keel adoption gaps, establish reproducible local tooling,
  preserve the approved design in canonical paths, and implement deterministic
  engine-aware display-state logic without connecting sensors or hardware.
- Acceptance:
  - valid local Git repository;
  - project-isolated PlatformIO entry point;
  - doctor, verifier, handoff verifier, playground, test-point ledger, and Git
    attributes exist and run;
  - all adoption conformance rows are resolved;
  - engine-stopped pressure never warns;
  - pressure/temperature boundaries and exact color stops are unit-tested;
  - native suite and complete firmware build pass in the current environment;
  - demo mode remains enabled and calibration remains invalid.
- Status: closed

## Slices

| Slice | Status | Test point result | Notes |
|---|---|---|---|
| 0.1 Git and canonical project structure | closed | pass | Valid `main` repository; reference checksums preserved |
| 0.2 Isolated toolchain and Keel scripts | verified | pass | PlatformIO Core 6.1.19; no global install |
| 0.3 Adopted design and BUILD-SPEC | verified | pass | No redesign or reference duplication |
| 0.4 RPM-aware display-state core | verified | 12/12 native tests | Pure C++ logic; RPM not displayed |
| 0.5 Gate zero and Sprint close | closed | pass | Full firmware build passed; no flash or vehicle test |

- Close-out evidence:
  - `./scripts/keel-doctor --check`: PASS.
  - `./scripts/pio test -e native`: 12/12 PASS.
  - `./scripts/pio run`: SUCCESS; RAM 7.5%, flash 5.5%.
  - `./scripts/keel-verify`: PASS after documentary self-audit.
  - host static compile: PASS.
  - suppression count: 0.
  - physical/hardware rows: explicitly unverified.
  - evidence commit: `a59e77c` (`chore: establish Sprint 0 foundation`).
  - closing commit: `93068f0` (`docs: close Sprint 0 remediation`).
  - continuation courier: verified with `VERDICT: CONTINUE` on 2026-07-30.
