## ADDED Requirements

### Requirement: API reads analog GPIO values
The controller SHALL provide an endpoint that reads an analog value from a supported analog-capable GPIO pin.

#### Scenario: Client reads analog value
- **WHEN** a client sends `GET /api/gpio/analog?pin=<n>` for a supported analog-capable GPIO
- **THEN** the controller reads the ADC value and returns a JSON response containing `ok: true`, the pin number, raw ADC value, resolution metadata, and millivolts when available

#### Scenario: Client reads analog value from unsupported pin
- **WHEN** a client requests an analog read for a missing, unsupported, or non-analog-capable GPIO
- **THEN** the controller rejects the request before touching hardware and returns a JSON error response

## MODIFIED Requirements

### Requirement: API lists supported GPIO pins
The controller SHALL provide an endpoint that lists GPIO pins available for REST control together with each pin's current parameters and capabilities, including whether the pin supports analog reads.

#### Scenario: Client lists GPIO pins
- **WHEN** a client sends `GET /api/gpio`
- **THEN** the controller returns a JSON response containing supported GPIO entries with pin number, current mode, pull-up state, pull-down state, value, input/output capabilities, and analog capability

#### Scenario: Reserved pins are excluded or marked unavailable
- **WHEN** the controller returns the GPIO list
- **THEN** pins reserved for boot, flash, I2C, UART, or board-critical functions are not offered as controllable pins or are explicitly marked unavailable

### Requirement: README documents GPIO REST usage
The repository README SHALL document the GPIO REST API endpoints, payloads, response shapes, and safety considerations.

#### Scenario: User reads project documentation
- **WHEN** a user opens `README.md`
- **THEN** the documentation explains how to list GPIOs, configure mode and pulls, read a digital GPIO, write an output, read an analog GPIO, and avoid unsafe pins

### Requirement: GPIO example is provided
The repository SHALL include a runnable GPIO REST example under `examples/gpio/`.

#### Scenario: User tries GPIO REST from Node.js
- **WHEN** a user opens the GPIO example directory
- **THEN** the example provides a Node.js script, package scripts, and documentation for listing, configuring, digitally reading, writing, and analog-reading GPIO pins through the REST API
