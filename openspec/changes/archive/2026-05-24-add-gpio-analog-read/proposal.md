## Why

Some controller GPIOs can read analog sensor values, but the current GPIO REST API only exposes digital reads and writes. Exposing analog reads through the same GPIO API makes simple sensors, potentiometers, and voltage-style test points usable from outside the firmware without custom controller code.

## What Changes

- Add analog-read support to the GPIO REST API for GPIOs that are explicitly marked as analog-capable.
- Extend GPIO pin metadata so clients can discover which pins support analog reads before accessing them.
- Return structured analog measurement data, including raw ADC value and resolution metadata.
- Update the repository README with the analog GPIO endpoint and safety notes.
- Extend the existing `examples/gpio/` Node.js example with an analog-read command and documentation.

## Capabilities

### New Capabilities

- None.

### Modified Capabilities

- `gpio-rest-api`: Adds analog-capable pin discovery and analog-read behavior to the existing GPIO REST API contract.

## Impact

- Firmware GPIO REST module: `src/gpio_rest_api.cpp` and `src/gpio_rest_api.h`
- Public HTTP API: new analog-read endpoint under the existing GPIO REST API
- Documentation: `README.md` and `examples/gpio/README.md`
- Example code: `examples/gpio/gpio.js` and package scripts if needed
