## ADDED Requirements

### Requirement: INA3221 example directory exists
The repository SHALL include an INA3221 example under `examples/ina3221/`.

#### Scenario: Reader browses examples
- **WHEN** a reader opens the repository examples directory
- **THEN** an `examples/ina3221/` subdirectory is available for INA3221 REST API usage

### Requirement: Example explains INA3221 setup assumptions
The INA3221 example SHALL document required assumptions such as controller IP, I2C address, channel numbering, shunt resistor values, wiring, and voltage/current safety limits.

#### Scenario: Reader prepares hardware
- **WHEN** a reader follows the INA3221 example
- **THEN** the example identifies hardware-specific values and safety limits before issuing REST requests

### Requirement: Example demonstrates I2C bus discovery
The INA3221 example SHALL show how to use the existing I2C REST scan endpoint to confirm the attached INA3221 is visible.

#### Scenario: User scans for the monitor
- **WHEN** a user wants to confirm the INA3221 is visible on the bus
- **THEN** the example includes a `GET /api/i2c/scan` request and explains common INA3221 address values

### Requirement: Example demonstrates INA3221 register reads
The INA3221 example SHALL show how to read INA3221 shunt-voltage and bus-voltage registers through the existing I2C REST API.

#### Scenario: User reads a measurement channel
- **WHEN** a user wants to read an INA3221 channel
- **THEN** the example uses existing I2C REST `write-read` requests to select and read the channel's shunt-voltage and bus-voltage registers

### Requirement: Example converts raw readings to engineering units
The INA3221 example SHALL explain how raw INA3221 register bytes map to bus voltage, shunt voltage, and optional current estimates.

#### Scenario: User interprets monitor output
- **WHEN** a user receives raw INA3221 measurement bytes
- **THEN** the example shows how to combine bytes and calculate bus volts, shunt millivolts, and current when a shunt resistance is supplied

### Requirement: Example includes runnable Node.js usage
The INA3221 example SHALL include a runnable Node.js file and package script support for scanning the bus and reading INA3221 channels.

#### Scenario: User automates INA3221 reads from Node.js
- **WHEN** a user wants to read the INA3221 from a Node.js script
- **THEN** the example provides an executable `ina3221.js`, a `package.json`, and instructions for scan and read commands

### Requirement: Example reads all monitor channels
The INA3221 Node.js example SHALL support reading one selected INA3221 channel or all three INA3221 channels.

#### Scenario: User reads the full monitor state
- **WHEN** a user runs the Node.js example in all-channel mode
- **THEN** the script reads and prints measurements for channels 1, 2, and 3

### Requirement: Example does not require firmware changes
The INA3221 example SHALL use the existing generic I2C REST API without requiring new firmware routes or build configuration changes.

#### Scenario: Developer adds the example
- **WHEN** the example is implemented
- **THEN** firmware source files and PlatformIO configuration remain unchanged
