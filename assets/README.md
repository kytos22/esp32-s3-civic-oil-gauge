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

The verified asset is 736×700 and loops indefinitely over 4.1 seconds. The
generator renders 205 samples at 50 FPS; GIF optimization stores 142 unique
frames, 132 of them at the exact 20 ms transition cadence, while combining only
identical holds. The 1,212,439-byte file has SHA-256
`1764a74750fcc07b492df7a9b1dbde7994e96181d40dfeab3faffad819e60f1c`.
