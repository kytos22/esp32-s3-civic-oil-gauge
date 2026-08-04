# Sprint 4 — Noncommercial project license

- Scope: allow people to study, modify and redistribute improvements to the
  project while withholding permission for commercial use.
- Acceptance:
  - the repository contains the unmodified official PolyForm Noncommercial 1.0.0
    text and a required copyright notice naming Marcos Vidal;
  - English and Spanish READMEs summarize permission to improve and redistribute
    for noncommercial purposes and the commercial-use restriction;
  - the documentation says this is source-available rather than OSI open source;
  - third-party dependencies and embedded tools remain under their own licenses;
  - the official text comparison, repository checks and remote publication pass.
- Status: complete — requested and published by Marcos on 2026-08-04

## Slices

| Slice | Status | Test point result | Notes |
| --- | --- | --- | --- |
| 4.1 Official terms and notice | complete | byte-identical SHA-256 match plus exact required notice | PolyForm text remains unmodified |
| 4.2 Bilingual public summary | complete | repository verifier passes | Improvements allowed for noncommercial purposes; commercial use excluded |
| 4.3 Publication verification | complete | remote Git blob IDs match local for license, notice and README | GitHub displays license family as `Other` |

## Verification evidence

- Official source:
  `https://raw.githubusercontent.com/polyformproject/polyform-licenses/1.0.0/PolyForm-Noncommercial-1.0.0.md`.
- Official and committed `LICENSE.md`: 4,563 bytes, SHA-256
  `c0ea4a896d2c8c394b29f9427589996db826cd501c512279ff0ed3ef48fabbe5`,
  byte-for-byte comparison passed.
- Required notice: `Required Notice: Copyright 2026 Marcos Vidal`.
- Remote/local Git blob matches: license `5ecc88cfc4b1cff608ed640efe913c9dd97935c3`,
  notice `bf70f3eb567939c370d8a4d42377bfff0c7a2de5`, README
  `cfd4965a156fb6ec01824066002744b02ab1bbbe`.
- `./scripts/keel-verify`: official-hash license gate and 31/31 acceptance
  coverage pass.
- GitHub reports license name `Other`, expected for this source-available
  noncommercial license; the committed full text is authoritative.
