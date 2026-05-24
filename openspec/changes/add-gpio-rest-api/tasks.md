## 1. Module Setup

- [ ] 1.1 Add `src/gpio_rest_api.h` declaring a `GpioRestApi::begin(WebServer &server)` entry point.
- [ ] 1.2 Add `src/gpio_rest_api.cpp` for GPIO REST route handlers and runtime pin state.
- [ ] 1.3 Register `GpioRestApi::begin(webServer)` during API-mode startup in `src/main.cpp`.
- [ ] 1.4 Keep GPIO REST implementation out of `src/i2c_rest_api.cpp/.h` except for shared patterns or independent helpers if needed.

## 2. GPIO Model and Validation

- [ ] 2.1 Define an explicit supported-pin table with per-pin input, output, pull-up, pull-down, and reserved capability metadata.
- [ ] 2.2 Implement helpers to find and validate supported pins from request input.
- [ ] 2.3 Implement mode parsing and normalization for `input`, `input_pullup`, `input_pulldown`, `output`, and any supported output variant.
- [ ] 2.4 Reject unsupported pins, reserved pins, invalid values, and unsupported mode/pull combinations before touching hardware.
- [ ] 2.5 Track REST-managed runtime state including mode, pull-up, pull-down, current value, and last output value.

## 3. REST Endpoints

- [ ] 3.1 Implement `GET /api/gpio` to list supported pins with current configuration and capability metadata.
- [ ] 3.2 Implement `GET /api/gpio/read?pin=<n>` to read one supported GPIO and return its current parameters.
- [ ] 3.3 Implement `POST /api/gpio/configure` to apply mode and pull configuration with optional initial output value.
- [ ] 3.4 Implement `POST /api/gpio/write` to write `0` or `1` to an already output-configured GPIO.
- [ ] 3.5 Return consistent JSON responses with `ok: true` on success and `ok: false` plus machine-readable `error` on validation or hardware failures.

## 4. Documentation and Verification

- [ ] 4.1 Update `README.md` with GPIO REST endpoint documentation, payload examples, response examples, and safe-pin guidance.
- [ ] 4.2 Add `examples/gpio/` with a dependency-free Node.js CLI, `package.json`, and README for list/configure/read/write operations.
- [ ] 4.3 Add serial logs for GPIO route startup, configuration, reads, writes, and rejected requests.
- [ ] 4.4 Build the firmware with PlatformIO.
- [ ] 4.5 Run a syntax check for the GPIO Node.js example.
- [ ] 4.6 If hardware is available, flash and monitor the controller.
- [ ] 4.7 If hardware is available, verify `GET /api/gpio` lists supported pins and current parameters.
- [ ] 4.8 If hardware is available, verify configure/read/write flows on a safe GPIO without disturbing I2C or provisioning behavior.
