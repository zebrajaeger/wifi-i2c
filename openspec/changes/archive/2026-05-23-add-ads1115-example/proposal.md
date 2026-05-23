## Why

The repository now has a runnable MCP4725 example, but there is no equivalent example for reading from a common ADC. An ADS1115 example will show how to use the existing I2C REST API for register-style write-then-read transactions.

## What Changes

- Add an `examples/ads1115/` subdirectory.
- Add a self-contained README for using an ADS1115 through the existing REST interface.
- Add a runnable Node.js example, plus a small `package.json`, equivalent in style to the MCP4725 example.
- Document how to scan for the ADS1115, configure a single-shot conversion, read the conversion register, and convert raw counts to voltage.
- Keep controller IP, I2C address, channel/gain/data-rate, and voltage assumptions clearly configurable.

## Capabilities

### New Capabilities
- `ads1115-rest-example`: Covers repository examples for using an ADS1115 ADC through the existing I2C REST API.

### Modified Capabilities

None.

## Impact

- Adds documentation/example files under `examples/ads1115/`.
- No firmware source, REST endpoint, request schema, dependency, or PlatformIO configuration changes are expected.
- The example depends on the existing I2C REST API documented by `i2c-rest-api`.
