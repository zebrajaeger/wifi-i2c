## Why

The GPIO REST API currently exposes pin-specific operations through query parameters such as `/api/gpio/adc?pin=34`. Path-style routes like `/api/gpio/pin/34/adc` are easier to read, compose, and mirror a resource-oriented REST shape.

## What Changes

- Add path-style GPIO routes under `/api/gpio/pin/<pin>/...` for pin-specific operations.
- Keep existing query-parameter endpoints working for compatibility.
- Support at least ADC reads through `/api/gpio/pin/<pin>/adc`, with the same validation and JSON response as `/api/gpio/adc?pin=<pin>`.
- Extend the pattern to digital read, digital write, configuration, and DAC write where it fits the existing GPIO API semantics.
- Update README and the GPIO Node.js example documentation to show the path-style routes.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `gpio-rest-api`: Add resource-oriented path-style routes for GPIO pin operations.

## Impact

- Firmware GPIO REST route registration and request parsing in `src/gpio_rest_api.cpp`.
- GPIO REST documentation in `README.md`.
- GPIO example documentation and optional command behavior under `examples/gpio/`.
- No new runtime dependencies are expected.
