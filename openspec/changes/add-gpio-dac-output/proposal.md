## Why

The ESP32 controller has two internal DAC outputs on GPIO25 and GPIO26, but the current GPIO REST API only exposes digital output and ADC input behavior. Exposing DAC writes through the GPIO block makes simple analog output tests possible without requiring an external MCP4725.

## What Changes

- Add internal DAC output support to the existing GPIO REST API for allowlisted DAC-capable pins.
- Extend GPIO pin metadata so clients can discover DAC-capable pins from `GET /api/gpio`.
- Add a DAC write endpoint that accepts 8-bit values from `0` through `255`.
- Track and report the last DAC output value for the current boot.
- Update `README.md` with DAC usage, limitations, and safety notes.
- Extend the existing `examples/gpio/` Node.js example with a DAC command and documentation.

## Capabilities

### New Capabilities

- None.

### Modified Capabilities

- `gpio-rest-api`: Adds DAC-capable pin discovery and internal DAC write behavior to the existing GPIO REST API contract.

## Impact

- Firmware GPIO REST module: `src/gpio_rest_api.cpp` and `src/gpio_rest_api.h`
- Public HTTP API: new DAC write endpoint under the existing GPIO REST API
- Documentation: `README.md` and `examples/gpio/README.md`
- Example code: `examples/gpio/gpio.js` and `examples/gpio/package.json`
