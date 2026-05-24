## Why

The controller can generate PWM signals, but the REST API currently exposes only digital GPIO, ADC, DAC, and pin configuration operations. Exposing PWM through the GPIO REST API enables hardware-in-the-loop tests to drive dimmers, servos, fans, and other PWM-controlled inputs without custom firmware changes.

## What Changes

- Add REST endpoints for starting, updating, reading, and stopping PWM output on supported GPIO pins.
- Use ESP32 hardware high-resolution timer/PWM support instead of software-timed toggling.
- Extend the GPIO list response with PWM capability and current PWM state metadata.
- Support both operation endpoints and path-style pin routes for PWM.
- Update README and the existing GPIO Node.js example to document and exercise PWM output.
- Preserve existing digital, ADC, DAC, and GPIO configuration behavior.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `gpio-rest-api`: Add PWM output control using hardware high-resolution timers.

## Impact

- Firmware GPIO REST implementation in `src/gpio_rest_api.cpp` and declarations in `src/gpio_rest_api.h` if needed.
- GPIO capability metadata and runtime state tracking.
- README GPIO REST documentation.
- Existing GPIO Node.js example under `examples/gpio/`.
- No new external runtime dependency is expected.
