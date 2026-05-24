## Context

The controller currently exposes I2C operations through a dedicated REST module that is started only after the controller connects to WiFi in station mode. Provisioning mode runs a captive portal and does not expose I2C REST routes. GPIO access should follow the same operational boundary: available in API mode, unavailable in provisioning mode.

GPIOs have more board-safety constraints than I2C operations. Some ESP32 pins are input-only, used for flash, boot strapping, UART, or I2C, and some can affect boot behavior. The implementation should therefore expose an explicit allowlist of supported pins rather than accepting any numeric GPIO.

## Goals / Non-Goals

**Goals:**
- Add a dedicated `gpio_rest_api.cpp/.h` module with clear ownership of GPIO REST behavior.
- Register GPIO REST routes alongside the existing I2C REST routes during normal WiFi station/API mode.
- Provide endpoints to list available GPIOs, read a GPIO, configure a GPIO, and write an output GPIO.
- Track and return current pin parameters: pin number, mode, pull-up, pull-down, current value, output capability, input capability, and last configured output value.
- Validate pin availability and invalid mode/pull combinations before calling GPIO hardware APIs.
- Return JSON responses with the same `ok: true/false` pattern used by the I2C API.
- Update the root `README.md` with GPIO REST endpoint documentation and safety notes.
- Add an `examples/gpio/` example with a dependency-free Node.js script and package scripts for listing, configuring, reading, and writing GPIOs.

**Non-Goals:**
- No PWM, analog read, interrupts, edge counting, debounce, pulse generation, or timing-sensitive GPIO features.
- No GPIO access in WiFi provisioning/captive portal mode.
- No automatic discovery of every ESP32 pad as available.
- No persistence of GPIO configuration across reboot unless a future change explicitly adds it.
- No changes to the existing I2C REST API contract.

## Decisions

1. Use a separate `GpioRestApi` module.

   This keeps responsibilities separated from `I2cRestApi` and mirrors the module boundary already used for WiFi provisioning. Alternative considered: adding GPIO routes to `i2c_rest_api.cpp`, but that would mix unrelated hardware responsibilities and make future maintenance messier.

2. Use an explicit supported-pin table.

   Each entry will define whether the GPIO can be used as input, output, pull-up, pull-down, and whether it should be considered reserved. The list endpoint returns this table with current runtime state. Alternative considered: accepting any ESP32 GPIO number, but that risks exposing flash, boot, I2C, or board-reserved pins.

3. Expose a compact REST surface under `/api/gpio`.

   Planned routes:
   - `GET /api/gpio` lists supported pins and current parameters.
   - `GET /api/gpio/read?pin=<n>` reads one supported pin.
   - `POST /api/gpio/configure` sets mode and pull configuration for one supported pin.
   - `POST /api/gpio/write` writes a digital value to a supported output pin.

   Alternative considered: one route per pin such as `/api/gpio/<pin>`, but the existing firmware uses `WebServer` handlers without path parameters, so query/body-based routing is simpler and consistent with the current style.

4. Model GPIO mode and pulls explicitly.

   Supported modes will be string values such as `input`, `input_pullup`, `input_pulldown`, `output`, and `output_open_drain` if the platform supports it cleanly. The response should normalize the applied state into fields like `mode`, `pullup`, and `pulldown`. Alternative considered: exposing raw Arduino constants, but strings are easier for REST clients and examples.

5. Keep writes separate from configuration.

   `POST /api/gpio/write` only changes output value and requires the pin to already be configured as an output-capable mode. `POST /api/gpio/configure` may optionally accept an initial output value when switching to output mode. This avoids accidental direction changes from a write call.

6. Use volatile runtime state.

   GPIO state is tracked in memory and initialized to safe defaults on boot. The API reports unknown/unconfigured pins as available but not actively configured until the user configures them. Alternative considered: storing GPIO config in preferences, but that can surprise users after reboot and should be a separate explicit feature.

7. Document and demonstrate the API in repository-facing artifacts.

   The root README should summarize the GPIO endpoints, payloads, and safety rules next to the existing I2C API documentation. A dedicated `examples/gpio/` folder should provide a runnable Node.js CLI so users can immediately list pins, configure a safe pin, read it, and write output values. Alternative considered: only documenting raw `Invoke-RestMethod` calls, but the existing examples have established a useful pattern for runnable Node.js examples.

## Risks / Trade-offs

- Exposing GPIOs can damage connected hardware if used incorrectly -> Use an allowlist, reject reserved pins, document mode/pull behavior, and keep writes explicit.
- Some pins are input-only or lack internal pulls -> Validate capabilities per pin and return JSON errors for unsupported operations.
- Board-specific safe pins may differ -> Keep the supported-pin table easy to adjust and include pin capability metadata in responses.
- Pull-up and pull-down support differs by chip/pin -> Normalize accepted modes and reject invalid combinations before applying pinMode.
- Runtime state can drift after reset -> The list endpoint reports current configured state for this boot only, and persistence remains out of scope.
- Users may copy examples against unsafe pins -> The README and example must point users at `GET /api/gpio` first and avoid hard-coding a pin without asking the user to choose a safe listed pin.
