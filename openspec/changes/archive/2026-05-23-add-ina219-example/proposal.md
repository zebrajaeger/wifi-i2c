## Why

Users can already access generic I2C devices through the controller REST API, and the repository now has concrete examples for several sensors. Adding an INA219 example gives the same ready-to-run path for a common single-channel current, voltage, and power monitor without changing firmware.

## What Changes

- Add an `examples/ina219/` directory with documentation for using the INA219 through the existing I2C REST API.
- Include a runnable Node.js example for scanning the bus and reading INA219 measurement registers.
- Document INA219 register access, common address assumptions, calibration considerations, conversion formulas, and safe usage notes.
- Keep the implementation limited to example files; no firmware routes, PlatformIO configuration, or REST API behavior changes.

## Capabilities

### New Capabilities
- `ina219-rest-example`: Documents and provides a runnable example for using an INA219 current, voltage, and power monitor through the existing I2C REST API.

### Modified Capabilities

None.

## Impact

- Adds documentation and Node.js example files under `examples/ina219/`.
- Adds an OpenSpec capability for the INA219 REST example.
- Uses existing REST endpoints such as `GET /api/i2c/scan` and register-style `POST /api/i2c/write-read`.
- No firmware source, REST route, dependency, or build configuration changes are required.
