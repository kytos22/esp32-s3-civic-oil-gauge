# Warning tone gate

## `WarningToneGate`

`include/warning_tone_gate.h` provides the renderer-independent transition gate
used to start and stop the demo warning-audio loop.

### `WarningToneCommand update(bool warningActive)`

- Input: whether the current evaluated pressure state is `warning`.
- Returns `startLoop` on a `false` to `true` transition.
- Returns `stopLoop` on a `true` to `false` transition.
- Returns `none` while the state remains unchanged.
- `warningActive()` exposes the retained state for deterministic tests.

Example:

```cpp
oilgauge::WarningToneGate gate;
gate.update(false);  // none
gate.update(true);   // startLoop
gate.update(true);   // none; worker keeps looping
gate.update(false);  // stopLoop
```

The class has no ESP-IDF dependency and is covered by the native Unity suite.
