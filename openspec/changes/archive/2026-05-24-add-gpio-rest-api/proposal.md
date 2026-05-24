## Why

The controller can already expose the I2C bus through REST, but external test automation also needs access to digital GPIOs for simple control and observation tasks. Adding a GPIO REST API lets clients list usable pins, configure pin modes and pulls, read inputs, and write outputs without flashing custom firmware for each test setup.

## What Changes

- Add a GPIO REST API that is available in the same WiFi station/API mode as the I2C REST API.
- Add an endpoint to list supported GPIO pins with their current configuration and value.
- Add endpoints to read one GPIO, configure one GPIO, and write one GPIO output value.
- Track per-pin configuration so mode, pull-up, pull-down, and last output value can be returned consistently.
- Validate pins and mode combinations before touching hardware.
- Implement the GPIO REST API in its own `src/gpio_rest_api.cpp/.h` module to keep responsibilities separate.
- Update the project README with GPIO REST usage.
- Add a runnable example under `examples/gpio/`.
- Keep provisioning portal behavior unchanged; GPIO REST routes are not exposed in captive portal/provisioning mode.

## Capabilities

### New Capabilities
- `gpio-rest-api`: Provides REST endpoints for listing, configuring, reading, and writing supported controller GPIO pins.

### Modified Capabilities

None.

## Impact

- Adds firmware source files for a dedicated GPIO REST module.
- Updates API-mode startup to register GPIO REST routes alongside the existing I2C routes.
- Updates `README.md` and adds GPIO example files under `examples/gpio/`.
- Adds JSON request validation and response behavior for GPIO operations.
- Does not change existing I2C REST API behavior or WiFi provisioning behavior.
