## Why

Users can already access generic I2C devices through the controller REST API, but there is no concrete example for the connected INA3221 three-channel current and bus-voltage monitor. A focused example makes it easy to verify the device on the bus and read useful measurements without changing firmware.

## What Changes

- Add an `examples/ina3221/` directory with documentation for using the INA3221 through the existing I2C REST API.
- Include a runnable Node.js example for scanning the bus and reading INA3221 channel measurements.
- Document INA3221 register access, common address assumptions, conversion formulas, and safe usage notes.
- Keep the implementation limited to example files; no firmware routes, PlatformIO configuration, or REST API behavior changes.

## Capabilities

### New Capabilities
- `ina3221-rest-example`: Documents and provides a runnable example for using an INA3221 current/voltage monitor through the existing I2C REST API.

### Modified Capabilities

None.

## Impact

- Adds documentation and Node.js example files under `examples/ina3221/`.
- Adds an OpenSpec capability for the INA3221 REST example.
- Uses existing REST endpoints such as `GET /api/i2c/scan` and register-style `POST /api/i2c/write-read`.
- No firmware source, REST route, dependency, or build configuration changes are required.
