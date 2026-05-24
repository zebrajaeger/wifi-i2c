## Context

The GPIO REST API already owns selected controller GPIO behavior, including digital reads/writes and analog reads. The current ESP32 target also provides two internal 8-bit DAC channels on GPIO25 and GPIO26. Those pins are already part of the GPIO allowlist, so DAC support should extend the existing GPIO block rather than becoming a separate I2C-style device API.

The DAC feature is intended for simple analog output tests and quick hardware-in-the-loop stimulus generation. It is not a replacement for higher-resolution external DAC devices such as MCP4725.

## Goals / Non-Goals

**Goals:**

- Expose ESP32 internal DAC writes through the existing GPIO REST module.
- Make DAC capability discoverable from `GET /api/gpio`.
- Accept DAC output values from `0` through `255`.
- Track the last DAC value written during the current boot.
- Update README and the existing `examples/gpio/` Node.js example.

**Non-Goals:**

- Add waveform generation, timed output, PWM, calibration, or voltage target conversion.
- Add DAC support for non-ESP32 boards that do not expose `dacWrite`.
- Change existing digital write, analog read, I2C, or WiFi provisioning behavior.
- Guarantee precision beyond the ESP32 internal DAC's coarse 8-bit output.

## Decisions

1. Add a dedicated `POST /api/gpio/dac` endpoint.

   Rationale: DAC writes are analog-output operations, not digital GPIO writes. A separate endpoint preserves the existing `/api/gpio/write` semantics and keeps validation straightforward.

   Alternative considered: Extend `/api/gpio/write` with a mode or value range. This would blur digital and DAC semantics and make accidental analog writes easier.

2. Add `dacCapable` metadata to the GPIO list.

   GPIO25 and GPIO26 will be marked DAC-capable. Clients can discover support from `GET /api/gpio` before attempting DAC output.

3. Use raw 8-bit values.

   The request payload will use `{ "pin": 25, "value": 0..255 }`. The response will include the normalized value and `resolutionBits: 8`, `maxRaw: 255`.

   Rationale: `dacWrite` uses 8-bit raw values. Voltage conversion depends on board supply, loading, and calibration, so clients should handle voltage mapping externally.

4. Keep DAC state separate from digital output state.

   The GPIO runtime state should track `lastDacValue` independently from `lastOutputValue`. DAC writes should not require configuring the pin as a digital output first.

5. Extend the existing GPIO example.

   The existing GPIO example already covers list, configure, digital read/write, and analog read. Adding a `dac` command keeps all GPIO-facing REST examples together.

## Risks / Trade-offs

- Internal DAC precision is limited -> Document that values are 8-bit raw DAC values and external precision DACs are better for accurate analog output.
- GPIO25/GPIO26 may be wired to external hardware -> Require allowlist validation and document that users must check wiring before driving DAC output.
- DAC writes may interact with previous digital use of the same pin -> Track DAC writes separately and document that DAC writes are analog output operations on DAC-capable pins.
- Non-ESP32 builds may not provide DAC support -> Guard implementation with platform support or fail gracefully if unavailable.
