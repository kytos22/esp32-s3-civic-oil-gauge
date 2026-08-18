# Keel Conformance — Civic ESP32 Oil Gauge

> Fresh sweep against Keel v5.15.1 `MANIFEST.md` Tables 1 and 3 on
> 2026-08-18. The embedded runtime is current; D-074 deliberately leaves two
> high-impact automation artifacts visible as missing instead of claiming an
> unverified reconciliation.

| Requirement | State | Where / condition | Evidence |
|---|---|---|---|
| `docs/PROGRESS.md` | present | `docs/PROGRESS.md` | Current project card, accepted baseline, open items and next action |
| `docs/decisions.md` | present | `docs/decisions.md` | Append-only decisions through D-074 |
| `docs/lessons-learned.md` | present | `docs/lessons-learned.md` | Includes measured triple-buffer failures L-012–L-015 |
| Off-machine durability | present | `origin` and project card | Public GitHub remote recorded; pushes remain manual |
| Clean tree at block close | present | Git plus `scripts/keel-verify` | Local commit required before hand-off |
| `CLAUDE.md` and root `AGENTS.md` lock | present | both files | Canonical lock stamped v5.15.1 |
| Gemini lock mirror | n/a | only when Gemini CLI is accepted | Codex is the only accepted tool |
| Embedded Keel trees | present | `.claude/skills/keel/`, `.agents/skills/keel/` | Both byte-match official v5.15.1 |
| Discovery artifacts | present | `docs/00-competitive-landscape.md`, `docs/01-discovery.md`, `docs/estimate.md`, `docs/token-ledger.md` | Adopted as-built and ledger current |
| Functional-spec artifacts | present | `docs/02-functional-spec.md`, `docs/03-technical-plan.md`, `docs/threat-model.md`, `docs/flows/` | Current platform, safety and flow contracts |
| `docs/budget.md` | n/a | `Client budget: no`; D-003 | No client quote |
| `docs/spec-references/` | n/a | only for recorded tests/code-to-port inputs | Visual inputs live under design references |
| `docs/rubrics/` | n/a | D-011 | No custom rubric accepted |
| Design references and hand-off | present | `docs/design/references/`, `docs/design/DESIGN-BRIEF.md`, `docs/design/design-handoff/` | Approved visual baseline preserved |
| Design Requests | n/a | required when first design gap appears | No open DR |
| Codex assistant rules | present | root and path-scoped `AGENTS.md` files | `Assistant config: rules` |
| Assistant subagents | n/a | requires `rules+agents` or `full`; D-008 | Checks run inline |
| Build contract | present | `docs/BUILD-SPEC.md` | State, visual and implementation contract |
| Repository controls | present | `.gitignore`, `.gitattributes`, Git repository | Generated state ignored and binaries marked |
| Sprint records | present | `docs/sprints/` | Phase 5 scope and evidence |
| Test-point matrix | present | `docs/05-test-points.md` | Stable AC IDs and coverage labels |
| API inventory | present | `docs/api/INDEX.md` | Progressive current public-header inventory |
| Conformance sweep | present | this file | Every current manifest category has a state |
| Playground | present | `docs/playground.md` | Headless software path and hardware boundary |
| `scripts/keel-verify` | present | executable | Paths, links, coverage and safety checks |
| `scripts/keel-doctor` | present | executable | Environment check and repair plan |
| Front-end minify/build script | n/a | no shipped JS/CSS pair | Simulator is a source/reference artifact |
| `scripts/keel-handoff-verify` | present | executable | Mechanical courier verification |
| `scripts/keel-close` | missing | D-074 | No canonical implementation; commit/merge/push automation needs a dedicated reviewed slice |
| `.githooks/post-commit` and `core.hooksPath` | present | `.githooks/post-commit`; local Git config | Hook executable and configured; real-commit firing is the close gate |
| `scripts/keel-stop-hook` and Stop registration | missing | D-074 | No canonical script; Codex Desktop exposes no project Stop-hook registration |
| `scripts/keel-session-pid.sh` | present | executable source helper | Stable PID plus process-start identity |
| Single-lane lock and chaining scripts | n/a | `Chaining: off`; D-009 | No launcher, lane or chain checker required |
| Chaining card fields | n/a | `Chaining: off` | No model/smoke state required |
| `.githooks/pre-commit` | n/a | accepted package is rules-only; D-008 | Confidential scan remains an explicit commit gate |
| Permission allow-lists | n/a | accepted package is rules-only | No unverified permissions committed |
| CI workflow | n/a | no accepted full assistant config/forge CI | Local verification is canonical |
| MCP registration | n/a | technical plan defines no dev MCP server | — |
| Phase 6 documentation set | n/a | project remains Phase 5 | Existing architecture/reference docs are maintained early |
| Root public READMEs | present | `README.md`, `README.es.md` | Reciprocal English/Spanish entry points |
| User guide/theme/brand marker | n/a | guide deferred until hardware-validated release candidate | Project card records deferral |
| Phase 7 release artifacts | n/a | no release candidate | — |
| Phase 8 website artifacts | n/a | `Website intent: no`; D-003 | — |
| Issue log | n/a | issue capture and duty off | No forge issue workflow accepted |
| Worktree slice reports | n/a | no worktree fan-out in this project | — |
| Archive index | present | `docs/old/README.md` | Historical originals indexed |
| Adoption audit | present | `docs/04-adoption-audit.md` | Proposed priorities retained |

## Table 3 reconciliation delta

| Version | State | Applied result |
|---|---|---|
| v5.4.x | present | Environment limits, inline verification and testability are recorded |
| v5.5.x | present | Manual autonomy, no notification, develop flow and issue duty off are recorded |
| v5.6.0 | present | Remote durability, clean-tree gate and lock item 4 are current |
| v5.7.0 | present | Continuation courier and resume verification are in place |
| v5.8.x | n/a | Issue capture and chaining are off |
| v5.9.0 | present | Issue capture remains explicitly off |
| v5.10.x | n/a | Manual autonomy retains `Chaining: off` |
| v5.11.0 | present | `pure-logic` test-first policy and Red-first coverage exist |
| v5.12.0 | n/a | Website intent is no |
| v5.13.0 | n/a | Chaining is off; chain smoke artifacts are conditional |
| v5.14.0 close executable | missing | D-074 defers `scripts/keel-close` pending reviewed implementation |
| v5.14.0 post-commit guard | present | Hook and local `core.hooksPath` installed; commit-time firing remains final evidence |
| v5.15.0 Stop hook | missing | D-074 records absent script and unavailable Codex registration surface |
| v5.15.1 session identity | present | `scripts/keel-session-pid.sh` installed |
| v5.15.1 Stop-hook regeneration | missing | Same D-074 automation slice; no v5.15.0 script exists to regenerate |

## Sweep result

- Embedded Keel source and lock portability: current at v5.15.1.
- Project/display documentation delta: reconciled.
- Missing: `scripts/keel-close`; `scripts/keel-stop-hook` plus a capable-tool
  registration and real firing proof.
- Keel reconciliation baseline therefore remains v5.13.0. The missing rows are
  deliberate visible blockers, not silent passes; D-074 defines their safe next
  slice.
