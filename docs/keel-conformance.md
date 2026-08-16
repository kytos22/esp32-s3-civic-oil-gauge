# Keel Conformance — Civic ESP32 Oil Gauge

> Keel v5.13.0 reconciliation sweep from `MANIFEST.md` Tables 1 and 3 on
> 2026-08-16. D-062 records the approved operating choices.

| Requirement | State | Where / decision | Notes |
|---|---|---|---|
| `docs/PROGRESS.md` | present | `docs/PROGRESS.md` | Full project card and executable next action |
| `docs/decisions.md` | present | `docs/decisions.md` | Append-only adopted/as-approved decisions |
| `docs/lessons-learned.md` | present | `docs/lessons-learned.md` | No closed lessons yet |
| Off-machine durability | present | `origin`; project-card `Durability:` | Public GitHub remote configured |
| Clean tree at block close | present | `develop`; `scripts/keel-verify` | Local commits required; manual mode gates pushes |
| `CLAUDE.md` + root `AGENTS.md` lock | present | both files | Same canonical v5.13.0 Keel block |
| Gemini lock mirror | n/a | condition: only if user works with Gemini CLI | Accepted tool is Codex only |
| Embedded Keel trees | present | `.claude/skills/keel/`, `.agents/skills/keel/` | Both verified byte-for-byte against installed v5.3.2; 39 files each |
| `docs/00-competitive-landscape.md` | present | file | Scan accepted |
| `docs/01-discovery.md` | present | file | Adopted as-built |
| `docs/estimate.md` | present | file | AI-time estimate; no client budget |
| `docs/token-ledger.md` | present | file | First row due at session close |
| `docs/02-functional-spec.md` | present | file | AC-01 through AC-23 |
| `docs/03-technical-plan.md` | present | file | Marked code map, change map, drivers, environment |
| `docs/threat-model.md` | present | file | Honest delivery states and omissions |
| `docs/flows/` | present | two flow files | Boot/display and calibration/cutover |
| `docs/budget.md` | n/a | condition: only if `Client budget: yes`; D-003 says no | — |
| `docs/spec-references/` | n/a | condition: only if spec records tests/code-to-port artifacts | Visual references are design inputs |
| `docs/rubrics/` | n/a | D-011 | No custom rubric accepted |
| `docs/design/references/` | present | `docs/design/references/` | Approved files moved without byte changes; SHA-256 matched |
| Codex assistant rules | present | `include/AGENTS.md`, `src/AGENTS.md`, `test/AGENTS.md` | `Assistant config: rules` |
| Assistant subagents | n/a | condition: only if `rules+agents` or `full`; D-008 | Codex checks run inline |
| `docs/design/DESIGN-BRIEF.md` | present | file; D-013 | Adopted exact brief, no creative redesign |
| `docs/design/design-handoff/` | present | orientation file; D-013 | No-Design legacy bridge; references are not duplicated |
| `docs/BUILD-SPEC.md` | present | file | State, token, accessibility, and implementation contract |
| `docs/design/design-requests/` | n/a | condition: when first Design Request appears | No open DR |
| `.gitignore` + `.gitattributes` | present | both files; valid local Git repository | Generated state and Keel courier ignored; binary assets marked |
| `docs/sprints/` | present | Sprint 0 closed; Sprint 1 fluid demo in progress | Phase 5 scope and evidence |
| `docs/05-test-points.md` | present | file | Stable AC IDs, Coverage and Red first columns; pre-policy rows marked `n/a — predates` |
| `docs/api/INDEX.md` | present | file | Complete core-header surface inventory |
| `docs/keel-conformance.md` | present | this file | Sweep contains every manifest row |
| `docs/playground.md` | present | file | Headless software recipe plus explicit hardware boundary |
| `scripts/keel-verify` | present | executable | Checks paths, links, conformance, AC coverage, safety invariants, and suppressions |
| `scripts/keel-doctor` | present | executable | Git/Python/C++/isolated PlatformIO checks and repair plan |
| Front-end minify/build script | n/a | condition: project ships front-end JS/CSS pairs | Prototype HTML is a design source, not shipped UI |
| `scripts/keel-handoff-verify` | present | executable | Five mechanical courier checks plus verdict |
| Single-lane lock | n/a | condition: `Chaining: start`; D-009 says off | — |
| `scripts/keel-continue` | n/a | condition: chaining not off; D-009 says off | — |
| `scripts/keel-chain-check` | n/a | condition: chaining not off; card says off | — |
| `Chaining model:` / `Chain verified:` | n/a | condition: chaining not off; card says off | — |
| `.githooks/pre-commit` | n/a | `Assistant config: rules`; D-008 defers full package | Reconsider after Git initialization |
| Permission allow-lists | n/a | `Assistant config: rules`; verified commands currently unavailable | No unverified permissions committed |
| CI workflow | n/a | condition: accepted full config and forge CI | No valid forge/repository |
| MCP registration | n/a | condition: plan defines dev MCP servers | None defined |
| `docs/architecture.md` | n/a | required from Phase 6; project is in Phase 5 | Existing `docs/ARCHITECTURE.md` is hardware-specific |
| `docs/api/`, `docs/usage/`, `docs/reference/` full layout | n/a | required from Phase 6 | API index progressive-backfill rule applies now |
| `docs/security.md` | n/a | required from Phase 6 | Threat model is current Phase 2 security artifact |
| `docs/accessibility.md` | n/a | required from Phase 6 | Embedded display verification still needs hardware |
| Root public READMEs | present | `README.md`, `README.es.md` | English base plus reciprocal Spanish entry point |
| `guide/` | n/a | required from Phase 6 unless declined; card says deferred | Revisit at release candidate |
| Guide theme/brand/meta marker | n/a | condition: user guide exists | — |
| `docs/07-release.md` | n/a | required from Phase 7 | No release candidate |
| Phase 8 site documents | n/a | condition: website intent; D-003 says no | — |
| Phase 8 launch report | n/a | condition: website launch | — |
| Phase 8 operations | n/a | condition: website launch | — |
| `docs/issues.md` | n/a | condition: forge issues accessed | No forge connected |
| Worktree slice reports | n/a | condition: fan-out over worktrees | This project has not fanned out over worktrees |
| `docs/old/` | present | `docs/old/README.md` | Archived Spanish originals are indexed |
| `docs/04-adoption-audit.md` | present | file | Proposed priorities pending user acceptance |

## Table 3 reconciliation delta

| Version | State | Applied result |
|---|---|---|
| v5.4.x | present | Environment limitations and inline verification are already recorded; no worktree fan-out applies |
| v5.5.x | present | D-062 records manual autonomy, no notification, develop flow and issue duty off; chaining remains off |
| v5.6.0 | present | Remote durability and clean-tree checks are recorded; canonical lock item 4 refreshed |
| v5.7.0 | present | Continuation courier is generated at every commit point and read first on resume; canonical lock item 2/5 refreshed |
| v5.8.x | n/a | Issue sweep/capture are off and chaining is off |
| v5.9.0 | present | D-062 records Issue capture off; no automatic queue or forge activity |
| v5.10.x | n/a | Manual autonomy retains `Chaining: off`; no launcher exists or is required |
| v5.11.0 | present | D-062 selects `pure-logic`; Red first column added and existing rows marked `n/a — predates` |
| v5.12.0 | n/a | Website intent is no |
| v5.13.0 | n/a | Chaining is off; chain checker/model/smoke artifacts are conditional |

## Sweep totals

- Present: 34 Table 1 rows plus 5 applicable Table 3 version groups
- Missing: 0
- Not applicable or not yet due: 27 Table 1 rows plus 5 Table 3 version groups
- Declined: 0

All current-position requirements and every post-v5.3.2 delta are present or
explicitly not applicable. Hardware and later-phase requirements remain gated.
