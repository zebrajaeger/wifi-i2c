## Context

The controller already exposes selected safe GPIOs through a dedicated GPIO REST module. That API supports listing pins, configuring digital input/output modes, reading digital values, and writing digital outputs. Some ESP32 GPIOs can also be used as ADC inputs, which is useful for simple analog sensors and hardware-in-the-loop measurements.

Analog reads should fit into the existing GPIO REST surface instead of introducing a separate analog subsystem. Clients should be able to discover analog-capable pins from `GET /api/gpio`, then read analog values through a dedicated endpoint.

## Goals / Non-Goals

**Goals:**

- Expose analog reads for explicitly allowlisted analog-capable GPIOs.
- Preserve the existing GPIO REST module ownership in `gpio_rest_api.cpp` and `gpio_rest_api.h`.
- Include analog capability metadata in the GPIO list response.
- Return machine-friendly analog data with raw ADC value and resolution metadata.
- Update README and the existing `examples/gpio/` Node.js example.

**Non-Goals:**

- Add analog output, PWM, DAC, calibration workflows, or continuous sampling.
- Expose every ADC-capable ESP32 pin automatically.
- Change the existing digital GPIO endpoints or I2C REST API behavior.
- Provide board-specific electrical safety guarantees beyond documenting the allowlist and validation behavior.

## Decisions

1. Add a dedicated `GET /api/gpio/analog?pin=<n>` endpoint.

   Rationale: Analog reads are semantically different from digital reads, and a dedicated endpoint keeps existing `/api/gpio/read` behavior stable. A query-based GET matches the existing digital read endpoint.

   Alternative considered: Extend `/api/gpio/read` with `type=analog`. This would reduce endpoint count, but would make validation and examples less clear.

2. Mark analog support in pin metadata.

   The pin list response will add an `analogCapable` field for each supported pin. Analog reads will only be accepted for pins marked `analogCapable: true`.

   Rationale: Clients can discover support dynamically and avoid hardcoding board assumptions.

3. Prefer ADC1-capable pins in the allowlist.

   ESP32 ADC2 pins can conflict with WiFi usage. Because this controller runs the REST API in WiFi station mode, analog-capable REST pins should be limited to safe ADC-capable GPIOs that work reliably while WiFi is active.

   Rationale: This keeps analog reads predictable in the controller's primary operating mode.

4. Return raw value plus resolution metadata.

   The analog response will include `pin`, `raw`, `resolutionBits`, and `maxRaw`. If platform support is available, it can also include `millivolts`.

   Rationale: Raw ADC values are portable and easy for clients to scale themselves, while resolution metadata makes the response self-describing.

5. Extend the existing GPIO example.

   The GPIO example already demonstrates listing, configuring, reading, and writing through the GPIO REST API. Adding an `analog` command there keeps GPIO usage in one place.

## Risks / Trade-offs

- ADC readings can be noisy or board-dependent -> Document that values are raw measurements and may need external scaling/filtering.
- Some ESP32 ADC pins are input-only -> Keep capability metadata separate for input, output, pull, and analog support.
- WiFi can interfere with ADC2 pins -> Prefer ADC1-capable pins for analog reads.
- Analog reads are not equivalent to digital configuration -> Keep analog reads separate from digital pin mode writes and avoid implicit digital output changes.
