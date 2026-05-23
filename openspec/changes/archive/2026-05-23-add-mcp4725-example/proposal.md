## Why

The I2C REST API is generic, but users need a concrete, copyable example that shows how a real I2C peripheral can be driven through it. An MCP4725 DAC example will make scan, write, and write-read style usage easier to understand without changing firmware behavior.

## What Changes

- Add a top-level `examples/` directory.
- Add an `examples/mcp4425/` subdirectory as requested, containing an MCP4725 usage example.
- Document how to discover the MCP4725 on the bus and how to write DAC output values through the existing REST interface.
- Include practical PowerShell and Node.js examples that use the current I2C REST endpoints.
- Keep the example self-contained and clearly mark controller IP, I2C address, and output voltage assumptions as environment-specific.

## Capabilities

### New Capabilities
- `mcp4725-rest-example`: Covers repository examples for using an MCP4725 DAC through the existing I2C REST API.

### Modified Capabilities

None.

## Impact

- Adds documentation/example files under `examples/mcp4425/`.
- No firmware code, REST endpoint, request schema, dependency, or build configuration changes are expected.
- The example depends on the existing I2C REST API documented by `i2c-rest-api`.
