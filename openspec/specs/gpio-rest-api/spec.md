# gpio-rest-api Specification

## Purpose
TBD - created by archiving change add-gpio-rest-api. Update Purpose after archive.
## Requirements
### Requirement: GPIO REST API uses a dedicated module
The controller SHALL implement GPIO REST behavior in dedicated `gpio_rest_api.cpp` and `gpio_rest_api.h` source files.

#### Scenario: Developer inspects firmware modules
- **WHEN** a developer looks for GPIO REST implementation
- **THEN** GPIO REST route registration, request validation, runtime pin state, and handlers are owned by the GPIO REST module rather than the I2C REST module

### Requirement: GPIO REST API starts in WiFi station mode
The controller SHALL start GPIO REST routes after it successfully connects to the configured WiFi network as a station.

#### Scenario: WiFi connection succeeds
- **WHEN** the controller boots with valid WiFi credentials and connects to the configured network
- **THEN** the controller starts HTTP REST routes for GPIO operations

#### Scenario: Controller enters provisioning mode
- **WHEN** the controller starts the provisioning access point
- **THEN** the controller does not expose GPIO REST API routes through the provisioning portal

### Requirement: API lists supported GPIO pins
The controller SHALL provide an endpoint that lists GPIO pins available for REST control together with each pin's current parameters and capabilities, including whether the pin supports ADC reads.

#### Scenario: Client lists GPIO pins
- **WHEN** a client sends `GET /api/gpio`
- **THEN** the controller returns a JSON response containing supported GPIO entries with pin number, current mode, pull-up state, pull-down state, value, input/output capabilities, and ADC capability

#### Scenario: Reserved pins are excluded or marked unavailable
- **WHEN** the controller returns the GPIO list
- **THEN** pins reserved for boot, flash, I2C, UART, or board-critical functions are not offered as controllable pins or are explicitly marked unavailable

### Requirement: API reads a GPIO pin
The controller SHALL provide an endpoint that reads the digital value of a supported GPIO pin.

#### Scenario: Client reads a configured input
- **WHEN** a client sends `GET /api/gpio/read?pin=<n>` for a supported readable GPIO
- **THEN** the controller reads the pin and returns its digital value and current pin parameters

#### Scenario: Client reads an unconfigured readable pin
- **WHEN** a client sends `GET /api/gpio/read?pin=<n>` for a supported readable GPIO that has not been configured through REST
- **THEN** the controller automatically configures the pin as plain `input` without pull-up or pull-down before returning the digital value and current pin parameters

#### Scenario: Client reads an invalid pin
- **WHEN** a client requests a missing, unsupported, or unreadable GPIO
- **THEN** the controller rejects the request before touching hardware and returns a JSON error response

### Requirement: API configures GPIO parameters
The controller SHALL provide an endpoint that configures mode and pull parameters for a supported GPIO pin.

#### Scenario: Client configures an input with pull-up
- **WHEN** a client sends `POST /api/gpio/configure` with a supported pin and mode `input_pullup`
- **THEN** the controller applies input pull-up mode and returns the normalized pin configuration

#### Scenario: Client configures an output
- **WHEN** a client sends `POST /api/gpio/configure` with a supported output-capable pin and mode `output`
- **THEN** the controller applies output mode and returns the normalized pin configuration

#### Scenario: Client submits invalid GPIO configuration
- **WHEN** a client sends `POST /api/gpio/configure` with an unsupported pin, unsupported mode, invalid pull combination, or capability mismatch
- **THEN** the controller rejects the request before applying the configuration and returns a JSON error response

### Requirement: API writes GPIO outputs
The controller SHALL provide an endpoint that writes a digital value to a supported output GPIO.

#### Scenario: Client writes an output value
- **WHEN** a client sends `POST /api/gpio/write` with a supported output-configured pin and value `0` or `1`
- **THEN** the controller writes the digital value and returns the updated pin parameters

#### Scenario: Client writes an unconfigured output-capable pin
- **WHEN** a client sends `POST /api/gpio/write` with a supported output-capable GPIO that has not been configured through REST
- **THEN** the controller automatically configures the pin as plain `output` without pull-up or pull-down, writes the requested value, and returns the updated pin parameters

#### Scenario: Client writes a non-output-capable pin
- **WHEN** a client sends `POST /api/gpio/write` for a pin that is unsupported or not output-capable
- **THEN** the controller rejects the request and returns a JSON error response

### Requirement: API tracks runtime GPIO state
The controller SHALL track each REST-managed GPIO pin's runtime configuration for the current boot.

#### Scenario: Client lists pins after configuration
- **WHEN** a client configures or writes a GPIO and then sends `GET /api/gpio`
- **THEN** the listed pin entry reflects the latest REST-managed mode, pull configuration, value, and last output value

### Requirement: API uses consistent JSON responses
The controller SHALL return JSON responses for GPIO REST API requests.

#### Scenario: Request succeeds
- **WHEN** a GPIO REST request succeeds
- **THEN** the response includes `ok: true` and operation-specific result data

#### Scenario: Request fails
- **WHEN** a GPIO REST request fails validation or encounters a hardware access error
- **THEN** the response includes `ok: false` and a machine-readable error field

### Requirement: README documents GPIO REST usage
The repository README SHALL document the GPIO REST API endpoints, payloads, response shapes, and safety considerations.

#### Scenario: User reads project documentation
- **WHEN** a user opens `README.md`
- **THEN** the documentation explains how to list GPIOs, configure mode and pulls, read a digital GPIO, write an output, read an ADC GPIO, and avoid unsafe pins

### Requirement: GPIO example is provided
The repository SHALL include a runnable GPIO REST example under `examples/gpio/`.

#### Scenario: User tries GPIO REST from Node.js
- **WHEN** a user opens the GPIO example directory
- **THEN** the example provides a Node.js script, package scripts, and documentation for listing, configuring, digitally reading, writing, and ADC-reading GPIO pins through the REST API

### Requirement: API reads ADC GPIO values
The controller SHALL provide an endpoint that reads an ADC value from a supported ADC-capable GPIO pin.

#### Scenario: Client reads ADC value
- **WHEN** a client sends `GET /api/gpio/adc?pin=<n>` for a supported ADC-capable GPIO
- **THEN** the controller reads the ADC value and returns a JSON response containing `ok: true`, the pin number, raw ADC value, resolution metadata, and millivolts when available

#### Scenario: Client reads ADC value from unsupported pin
- **WHEN** a client requests an ADC read for a missing, unsupported, or non-ADC-capable GPIO
- **THEN** the controller rejects the request before touching hardware and returns a JSON error response
