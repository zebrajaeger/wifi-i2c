## ADDED Requirements

### Requirement: INA219 example directory exists
The repository SHALL include an INA219 example under `examples/ina219/`.

#### Scenario: Reader browses examples
- **WHEN** a reader opens the repository examples directory
- **THEN** an `examples/ina219/` subdirectory is available for INA219 REST API usage

### Requirement: Example explains INA219 setup assumptions
The INA219 example SHALL document required assumptions such as controller IP, I2C address, shunt resistor value, current LSB, wiring, and voltage/current safety limits.

#### Scenario: Reader prepares hardware
- **WHEN** a reader follows the INA219 example
- **THEN** the example identifies hardware-specific values and safety limits before issuing REST requests

### Requirement: Example demonstrates I2C bus discovery
The INA219 example SHALL show how to use the existing I2C REST scan endpoint to confirm the attached INA219 is visible.

#### Scenario: User scans for the monitor
- **WHEN** a user wants to confirm the INA219 is visible on the bus
- **THEN** the example includes a `GET /api/i2c/scan` request and explains common INA219 address values

### Requirement: Example demonstrates INA219 register reads
The INA219 example SHALL show how to read INA219 shunt-voltage, bus-voltage, current, and power registers through the existing I2C REST API.

#### Scenario: User reads monitor registers
- **WHEN** a user wants to read INA219 measurements
- **THEN** the example uses existing I2C REST `write-read` requests to select and read INA219 measurement registers

### Requirement: Example demonstrates optional calibration
The INA219 example SHALL explain and support optional calibration writes before reading current and power registers.

#### Scenario: User requests calibrated measurements
- **WHEN** a user runs the Node.js example with calibration enabled
- **THEN** the script writes the INA219 calibration register through the existing I2C REST write endpoint before reading current and power

### Requirement: Example converts raw readings to engineering units
The INA219 example SHALL explain how raw INA219 register bytes map to bus voltage, shunt voltage, current, and power.

#### Scenario: User interprets monitor output
- **WHEN** a user receives raw INA219 measurement bytes
- **THEN** the example shows how to combine bytes and calculate bus volts, shunt millivolts, current, and power using the selected calibration values

### Requirement: Example includes runnable Node.js usage
The INA219 example SHALL include a runnable Node.js file and package script support for scanning the bus and reading INA219 measurements.

#### Scenario: User automates INA219 reads from Node.js
- **WHEN** a user wants to read the INA219 from a Node.js script
- **THEN** the example provides an executable `ina219.js`, a `package.json`, and instructions for scan and read commands

### Requirement: Example does not require firmware changes
The INA219 example SHALL use the existing generic I2C REST API without requiring new firmware routes or build configuration changes.

#### Scenario: Developer adds the example
- **WHEN** the example is implemented
- **THEN** firmware source files and PlatformIO configuration remain unchanged
