# Warning tone gate

## `WarningToneGate`

`include/warning_tone_gate.h` provides the renderer-independent transition gate
used by the demo audio path.

### `bool update(bool warningActive)`

- Input: whether the current evaluated pressure state is `warning`.
- Returns `true` only when the input changes from `false` to `true`.
- Returns `false` while warning remains active and while it is inactive.
- A `false` update re-arms the next rising edge.

Example:

```cpp
oilgauge::WarningToneGate gate;
gate.update(false);  // false
gate.update(true);   // true: request one tone
gate.update(true);   // false: do not repeat
gate.update(false);  // false: re-arm
```

The class has no ESP-IDF dependency and is covered by the native Unity suite.
