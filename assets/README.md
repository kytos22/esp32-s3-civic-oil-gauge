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
generator renders 205 samples at 50 FPS; GIF optimization stores 148 unique
frames, 136 of them at the exact 20 ms transition cadence, while combining only
identical holds. The 1,238,704-byte file has SHA-256
`0379ef71618ba94524497442d9f0fce812899563747b50e97588f4fcedb3cb80`.
The generator inlines the simulator's two current icon masks before headless
capture so browser `file://` restrictions cannot silently omit them.
