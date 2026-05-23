# mcp4725-rest-example Specification

## Purpose
TBD - created by archiving change add-mcp4725-example. Update Purpose after archive.
## Requirements
### Requirement: MCP4725 example directory exists
The repository SHALL include an MCP4725 example under `examples/mcp4425/`.

#### Scenario: Reader browses examples
- **WHEN** a reader opens the repository examples directory
- **THEN** an `examples/mcp4425/` subdirectory is available for MCP4725 REST API usage

### Requirement: Example explains MCP4725 setup assumptions
The MCP4725 example SHALL document required assumptions such as controller IP, I2C address, wiring, and DAC supply voltage.

#### Scenario: Reader prepares hardware
- **WHEN** a reader follows the MCP4725 example
- **THEN** the example identifies which values are placeholders or hardware-specific before issuing REST requests

### Requirement: Example demonstrates I2C bus discovery
The MCP4725 example SHALL show how to use the existing I2C REST scan endpoint to discover attached devices.

#### Scenario: User scans for the DAC
- **WHEN** a user wants to confirm the MCP4725 is visible on the bus
- **THEN** the example includes a `GET /api/i2c/scan` request and explains how to interpret the discovered address list

### Requirement: Example demonstrates writing DAC values
The MCP4725 example SHALL show how to write MCP4725 DAC output values through the existing I2C REST write endpoint.

#### Scenario: User sets a DAC output
- **WHEN** a user wants to set an MCP4725 output value
- **THEN** the example includes a `POST /api/i2c/write` request with the bytes needed for a 12-bit MCP4725 DAC value

### Requirement: Example includes runnable Node.js usage
The MCP4725 example SHALL include a runnable Node.js file and package script support for scanning the bus and writing an MCP4725 DAC value through the REST API.

#### Scenario: User automates DAC writes from Node.js
- **WHEN** a user wants to drive the MCP4725 from a Node.js script
- **THEN** the example provides an executable `mcp4725.js`, a `package.json`, and instructions for scan and write commands

### Requirement: Example does not require firmware changes
The MCP4725 example SHALL use the existing generic I2C REST API without requiring new firmware routes or build configuration changes.

#### Scenario: Developer adds the example
- **WHEN** the example is implemented
- **THEN** firmware source files and PlatformIO configuration remain unchanged

