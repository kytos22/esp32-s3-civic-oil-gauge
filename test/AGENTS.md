# Scope: automated test code in this directory

Sources of truth: `docs/02-functional-spec.md` acceptance criteria,
`docs/03-technical-plan.md` testing section, and the root `AGENTS.md`.

- Name or annotate tests with the stable `AC-nn` criterion they prove.
- Cover valid, boundary, invalid, missing-calibration, and sensor-fault paths.
- Alarm tests must include engine stopped, engine running, reduced-motion,
  and threshold boundaries once the RPM-aware API exists.
- Never weaken an assertion or skip a test to make the suite green.
- Every production bug fix gets a regression test in the same change.
- Keep native core tests hardware-independent and deterministic.
- Hardware-only legs remain explicitly tagged `HARDWARE`; do not report them
  as passing from simulation.
- Record the exact command and result at each Keel test point.
