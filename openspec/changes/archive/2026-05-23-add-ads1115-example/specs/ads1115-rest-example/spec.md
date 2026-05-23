## ADDED Requirements

### Requirement: ADS1115 example directory exists
The repository SHALL include an ADS1115 example under `examples/ads1115/`.

#### Scenario: Reader browses examples
- **WHEN** a reader opens the repository examples directory
- **THEN** an `examples/ads1115/` subdirectory is available for ADS1115 REST API usage

### Requirement: Example explains ADS1115 setup assumptions
The ADS1115 example SHALL document required assumptions such as controller IP, I2C address, input channel, gain, data rate, wiring, and voltage limits.

#### Scenario: Reader prepares hardware
- **WHEN** a reader follows the ADS1115 example
- **THEN** the example identifies hardware-specific values and safety limits before issuing REST requests

### Requirement: Example demonstrates I2C bus discovery
The ADS1115 example SHALL show how to use the existing I2C REST scan endpoint to discover attached devices.

#### Scenario: User scans for the ADC
- **WHEN** a user wants to confirm the ADS1115 is visible on the bus
- **THEN** the example includes a `GET /api/i2c/scan` request and explains the common ADS1115 address range

### Requirement: Example demonstrates register-style ADC reads
The ADS1115 example SHALL show how to configure an ADS1115 conversion and read the conversion register through the existing I2C REST API.

#### Scenario: User reads a channel
- **WHEN** a user wants to read an ADS1115 analog input
- **THEN** the example uses existing I2C REST requests to write the configuration register and read the conversion register

### Requirement: Example converts raw readings to voltage
The ADS1115 example SHALL explain how the signed raw ADS1115 reading maps to an approximate voltage for the selected full-scale range.

#### Scenario: User interprets ADC output
- **WHEN** a user receives raw ADS1115 bytes
- **THEN** the example shows how to combine the bytes, sign-extend the value, and calculate approximate volts

### Requirement: Example includes runnable Node.js usage
The ADS1115 example SHALL include a runnable Node.js file and package script support for scanning the bus and reading an ADS1115 channel.

#### Scenario: User automates ADC reads from Node.js
- **WHEN** a user wants to read the ADS1115 from a Node.js script
- **THEN** the example provides an executable `ads1115.js`, a `package.json`, and instructions for scan and read commands

### Requirement: Example does not require firmware changes
The ADS1115 example SHALL use the existing generic I2C REST API without requiring new firmware routes or build configuration changes.

#### Scenario: Developer adds the example
- **WHEN** the example is implemented
- **THEN** firmware source files and PlatformIO configuration remain unchanged
