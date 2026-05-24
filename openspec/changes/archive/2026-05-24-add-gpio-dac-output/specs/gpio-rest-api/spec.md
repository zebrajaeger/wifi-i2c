## ADDED Requirements

### Requirement: API writes internal DAC outputs
The controller SHALL provide an endpoint that writes an 8-bit value to a supported internal DAC-capable GPIO pin.

#### Scenario: Client writes DAC value
- **WHEN** a client sends `POST /api/gpio/dac` with a supported DAC-capable GPIO and value `0` through `255`
- **THEN** the controller writes the DAC value and returns a JSON response containing `ok: true`, pin number, written value, resolution metadata, and current pin parameters

#### Scenario: Client writes DAC value to unsupported pin
- **WHEN** a client sends `POST /api/gpio/dac` for a missing, unsupported, or non-DAC-capable GPIO
- **THEN** the controller rejects the request before touching hardware and returns a JSON error response

#### Scenario: Client submits invalid DAC value
- **WHEN** a client sends `POST /api/gpio/dac` with a value outside `0` through `255` or a non-integer value
- **THEN** the controller rejects the request before touching hardware and returns a JSON error response

## MODIFIED Requirements

### Requirement: API lists supported GPIO pins
The controller SHALL provide an endpoint that lists GPIO pins available for REST control together with each pin's current parameters and capabilities, including whether the pin supports ADC reads or internal DAC writes.

#### Scenario: Client lists GPIO pins
- **WHEN** a client sends `GET /api/gpio`
- **THEN** the controller returns a JSON response containing supported GPIO entries with pin number, current mode, pull-up state, pull-down state, value, input/output capabilities, ADC capability, DAC capability, and last DAC value when available

#### Scenario: Reserved pins are excluded or marked unavailable
- **WHEN** the controller returns the GPIO list
- **THEN** pins reserved for boot, flash, I2C, UART, or board-critical functions are not offered as controllable pins or are explicitly marked unavailable

### Requirement: API tracks runtime GPIO state
The controller SHALL track each REST-managed GPIO pin's runtime configuration and last written output values for the current boot.

#### Scenario: Client lists pins after configuration
- **WHEN** a client configures, digitally writes, or DAC-writes a GPIO and then sends `GET /api/gpio`
- **THEN** the listed pin entry reflects the latest REST-managed mode, pull configuration, digital value, last digital output value, and last DAC value when available

### Requirement: README documents GPIO REST usage
The repository README SHALL document the GPIO REST API endpoints, payloads, response shapes, and safety considerations.

#### Scenario: User reads project documentation
- **WHEN** a user opens `README.md`
- **THEN** the documentation explains how to list GPIOs, configure mode and pulls, read a digital GPIO, write an output, read an ADC GPIO, write an internal DAC output, and avoid unsafe pins

### Requirement: GPIO example is provided
The repository SHALL include a runnable GPIO REST example under `examples/gpio/`.

#### Scenario: User tries GPIO REST from Node.js
- **WHEN** a user opens the GPIO example directory
- **THEN** the example provides a Node.js script, package scripts, and documentation for listing, configuring, digitally reading, writing, ADC-reading, and DAC-writing GPIO pins through the REST API
