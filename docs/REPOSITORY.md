# Repository organization and publication contract

## Reference scope

The public organization was informed by
[`kytos22/esp32-s3-civic-boost-gauge`](https://github.com/kytos22/esp32-s3-civic-boost-gauge)
at commit `9ca43404304388c5f7f489296d08e9e89c59f64a`. Only repository-level patterns
were considered; no turbo-gauge source code, sensor values, settings, firmware,
diagrams, caches, visual assets or release data are inputs to this project.

## Patterns adopted

- English and Spanish root entry points.
- A product-first README with preview, features, hardware, wiring status, build
  instructions, firmware downloads and contributor guidance.
- A dedicated `assets/` namespace for project-owned source visuals.
- Immutable `firmware/<version>/` release packages with bilingual instructions,
  complete and application-only images, visual proof and checksums.
- Practical technical documentation kept separately from release packages.

## Patterns deliberately not adopted

- `GOLDEN_VERSION.md` or any equivalent golden snapshot. The renderer remains
  governed by its design source, tests and measured evidence.
- Bundled copies of Arduino, LVGL or board libraries. ESP-IDF component manifests
  and `dependencies.lock` are the authoritative dependency mechanism.
- Wokwi files or a simulated wiring diagram that cannot model the actual CO5300
  display and Innovate signal path faithfully.
- Turbo-specific generated caches, logos, sensor diagrams, settings or packaging
  scripts.
- Committed build directories, PlatformIO caches, managed component downloads,
  local serial logs or device identifiers.

## Maintained top-level map

```text
README.md / README.es.md   Public project entry points
assets/                    Oil-gauge-owned visual sources
firmware/                  Validated immutable release packages
docs/                      Maintained engineering and safety documentation
include/                   Public firmware interfaces
src/                       ESP-IDF firmware implementation
test/                      Native Unity tests
scripts/                   Pinned build and verification commands
```

Keel's portability files remain present because they protect calibration,
vehicle and release workflows across development tools. They are project
governance, not firmware data.

## Publication gate

Before any remote is created or pushed:

1. The user chooses the repository visibility explicitly.
2. The current snapshot contains no secrets, personal data or exact device
   identifiers.
3. Generated and local-only paths remain ignored and untracked.
4. Native tests, the complete ESP-IDF build and `./scripts/keel-verify` pass.
5. A public repository starts from a sanitized snapshot rather than the current
   development history, because earlier local commits contain an exact test-board
   identifier in historical evidence.

Marcos selected public visibility on 2026-08-04. The repository name is
`kytos22/esp32-s3-civic-oil-gauge`; publication must use the sanitized
fresh-history snapshot described above.
