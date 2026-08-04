# Product assets

This directory is reserved for oil-gauge-owned visual source assets and public
previews. It must never contain assets copied from the Civic boost-gauge project.

The currently approved renderer reference remains under
[`docs/design/references/`](../docs/design/references/) because it is also the
binding design input. A release may copy a verified screen capture into its own
`firmware/<version>/` package, but generated release evidence is not a source
asset.

`oil-gauge-demo.gif` is the README preview generated from the approved standalone
HTML by `scripts/generate-readme-demo-gif.py`. It contains only synthetic demo
states and remains linked to the live simulator. Regenerate it after an approved
simulator visual change; do not edit its pixels independently.

The verified asset is 736×700, nine frames, 245,667 bytes, loops indefinitely and
has SHA-256 `e590665c3363fae2a3d0095b07fc61a6c1dd5ebe3f3dc67ac45441232ccf3583`.
