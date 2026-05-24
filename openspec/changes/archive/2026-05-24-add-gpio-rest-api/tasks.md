## 1. Module Setup

- [x] 1.1 Add `src/gpio_rest_api.h` declaring a `GpioRestApi::begin(WebServer &server)` entry point.
- [x] 1.2 Add `src/gpio_rest_api.cpp` for GPIO REST route handlers and runtime pin state.
- [x] 1.3 Register `GpioRestApi::begin(webServer)` during API-mode startup in `src/main.cpp`.
- [x] 1.4 Keep GPIO REST implementation out of `src/i2c_rest_api.cpp/.h` except for shared patterns or independent helpers if needed.

## 2. GPIO Model and Validation

- [x] 2.1 Define an explicit supported-pin table with per-pin input, output, pull-up, pull-down, and reserved capability metadata.
- [x] 2.2 Implement helpers to find and validate supported pins from request input.
- [x] 2.3 Implement mode parsing and normalization for `input`, `input_pullup`, `input_pulldown`, `output`, and any supported output variant.
- [x] 2.4 Reject unsupported pins, reserved pins, invalid values, and unsupported mode/pull combinations before touching hardware.
- [x] 2.5 Track REST-managed runtime state including mode, pull-up, pull-down, current value, and last output value.

## 3. REST Endpoints

- [x] 3.1 Implement `GET /api/gpio` to list supported pins with current configuration and capability metadata.
- [x] 3.2 Implement `GET /api/gpio/read?pin=<n>` to read one supported GPIO and return its current parameters.
- [x] 3.3 Implement `POST /api/gpio/configure` to apply mode and pull configuration with optional initial output value.
- [x] 3.4 Implement `POST /api/gpio/write` to write `0` or `1` to an already output-configured GPIO.
- [x] 3.5 Return consistent JSON responses with `ok: true` on success and `ok: false` plus machine-readable `error` on validation or hardware failures.

## 4. Documentation and Verification

- [x] 4.1 Update `README.md` with GPIO REST endpoint documentation, payload examples, response examples, and safe-pin guidance.
- [x] 4.2 Add `examples/gpio/` with a dependency-free Node.js CLI, `package.json`, and README for list/configure/read/write operations.
- [x] 4.3 Add serial logs for GPIO route startup, configuration, reads, writes, and rejected requests.
- [x] 4.4 Build the firmware with PlatformIO.
- [x] 4.5 Run a syntax check for the GPIO Node.js example.
- [x] 4.6 If hardware is available, flash and monitor the controller.
- [x] 4.7 If hardware is available, verify `GET /api/gpio` lists supported pins and current parameters.
- [x] 4.8 If hardware is available, verify configure/read/write flows on a safe GPIO without disturbing I2C or provisioning behavior.

## 5. Follow-up Refinements

- [x] 5.1 Remove PlatformIO configuration that emits startup warnings during build or monitor commands.
- [x] 5.2 Auto-configure unconfigured readable pins as plain `input` on `GET /api/gpio/read`.
- [x] 5.3 Auto-configure unconfigured output-capable pins as plain `output` on `POST /api/gpio/write`.
- [x] 5.4 Rebuild firmware and run the GPIO example syntax check.
- [x] 5.5 If hardware is available, flash and verify auto-read, auto-write, and warning-free command output.
